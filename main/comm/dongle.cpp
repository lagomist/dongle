#include "dongle.h"
#include "gatt_client.h"
#include "usb_cli.h"
#include "timer.h"
#include "nrf_delay.h"
#define NRF_LOG_MODULE_NAME Dongle
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

namespace dongle {

constexpr static uint8_t const SCAN_MAX_BUF_NUM = 30;
static Wrapper::AppTimer::Timer _timer_handle;
static Wrapper::BLE::Client::EvtType _status;
static Wrapper::BLE::Client::AdvReport _scan_dev_list[SCAN_MAX_BUF_NUM];
static Wrapper::BLE::Client::AdvReport _target_device;
static Wrapper::BLE::Client::CharHandle _char_handle;
static Wrapper::BLE::Client::GattDatabase* _database = nullptr;

static int database_find_chars(uint16_t char_uuid, Wrapper::BLE::Client::CharHandle &chars) {
	if (_database == nullptr) return -1;
	for (int i = 0; i < _database->service_count; i++) {
		for (int j = 0; j < _database->services[i].char_count; j++) {
			if (_database->services[i].characteristics[j].uuid == char_uuid) {
				chars.char_handle = _database->services[i].characteristics[j].handle;
				chars.cccd_handle = _database->services[i].characteristics[j].cccd_handle;
				chars.uuid = char_uuid;
				return 0;
			}
		}
	}
	return -2;
}

static void ble_evt_callback(Wrapper::BLE::Client::EvtType evt, uint16_t handle) {
	_status = evt;
	_timer_handle.restart();
}

static void scan_callback(Wrapper::BLE::Client::AdvReport report) {
	if (strlen(report.name) <= 0) return;

	for (int i = 0; i < SCAN_MAX_BUF_NUM; i++) {
		if (strcmp(_scan_dev_list[i].name, report.name) == 0) break;
		if (_scan_dev_list[i].addr[0] == 0) {
			_scan_dev_list[i] = report;
			break;
		}
	}
}

static void db_callback(Wrapper::BLE::Client::GattDatabase* database) {
	_database = database;
	_status = Wrapper::BLE::Client::EvtType::SERVICE_DISCOVER_EVT;
	_timer_handle.restart();
}

static void ble_recv_callback(uint16_t handle, const uint8_t *data, uint16_t len) {
	usb_cli::write({(char *)data, (size_t )len});
}

static void dongle_task(void *arg) {
	usb_cli::option_printf("\nBle %s\n", Wrapper::BLE::Client::evt_to_str(_status).data());
	switch (_status) {
	case Wrapper::BLE::Client::EvtType::SCAN_TIMEOUT_EVT:
		usb_cli::write("found device:\n");
		for (int i = 0; (i < SCAN_MAX_BUF_NUM) && (_scan_dev_list[i].addr[0] != 0); i++) {
			usb_cli::write("%-24s\t%02X:%02X:%02X:%02X:%02X:%02X\t%d\n",
							_scan_dev_list[i].name,
							_scan_dev_list[i].addr[5], _scan_dev_list[i].addr[4], _scan_dev_list[i].addr[3],
							_scan_dev_list[i].addr[2], _scan_dev_list[i].addr[1], _scan_dev_list[i].addr[0],
							_scan_dev_list[i].rssi);
		}
		break;
	case Wrapper::BLE::Client::EvtType::CONNECT_TIMEOUT_EVT:
		usb_cli::write("%s connect failed.\n", _target_device.name);
		break;
	case Wrapper::BLE::Client::EvtType::CONNECTED_EVT:
		usb_cli::write("%s connected.\n", _target_device.name);
		break;
	case Wrapper::BLE::Client::EvtType::SERVICE_DISCOVER_EVT:
		for (int srv_index = 0; srv_index < _database->service_count; srv_index++) {
			Wrapper::BLE::Client::CharacteristicProperty *db_chars = _database->services[srv_index].characteristics;
			usb_cli::write("Service UUID: %04X\n", _database->services[srv_index].uuid);
			for (int char_index = 0; char_index < _database->services[srv_index].char_count; char_index++) {
				usb_cli::write("Characteristic UUID: %04X", db_chars[char_index].uuid);
				usb_cli::write("\tproperties: %02X\n", db_chars[char_index].properties);
			}
		}
		break;
	default:
		break;
	}
}

void ble_scan(uint16_t timeout) {
	_status = Wrapper::BLE::Client::EvtType::IDLE;
	memset(_scan_dev_list, 0, sizeof(_scan_dev_list));
	Wrapper::BLE::Client::scan_start(timeout);
}

int ble_connect(std::string_view name, uint16_t timeout) {
	memset(&_char_handle, 0, sizeof(_char_handle));
	for (int i = 0; (i < SCAN_MAX_BUF_NUM) && (_scan_dev_list[i].addr[0] != 0); i++) {
		if (strcmp(_scan_dev_list[i].name, name.data()) == 0) {
			_target_device = _scan_dev_list[i];
			return Wrapper::BLE::Client::connection(_scan_dev_list[i].addr, timeout);
		}
	}
	return -1;
}

int ble_select(uint16_t char_uuid) {
	if (database_find_chars(char_uuid, _char_handle) < 0) {
		return -1;
	}
	return Wrapper::BLE::Client::notif_config(_char_handle.cccd_handle, true);
}

int ble_send(std::string_view buf) {
	if (_char_handle.char_handle == 0x0000) {
		return -1;
	}
	return Wrapper::BLE::Client::send(_char_handle.char_handle, buf.data(), buf.size());
}

int ble_disconnect() {
	return Wrapper::BLE::Client::disconnection();
}

int init() {
	Wrapper::BLE::Client::register_evt_callback(ble_evt_callback);
	Wrapper::BLE::Client::register_scan_callback(scan_callback);
	Wrapper::BLE::Client::register_db_callback(db_callback);
	Wrapper::BLE::Client::register_recv_callback(ble_recv_callback);
	usb_cli::enable();
	_timer_handle.create(dongle_task, Wrapper::AppTimer::CALL_IMMEDIATE);
	return 0;
}


}
