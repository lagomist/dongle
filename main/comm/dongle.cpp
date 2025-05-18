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
constexpr static uint8_t const CHAR_MAX_BUF_NUM = 6;
static Wrapper::AppTimer::Timer _timer_handle;
static Wrapper::BLE::Client::EvtType _status;
static Wrapper::BLE::Client::AdvReport _scan_dev_list[SCAN_MAX_BUF_NUM];
static Wrapper::BLE::Client::AdvReport _target_device;
static Wrapper::BLE::Client::CharHandle _char_handle[CHAR_MAX_BUF_NUM];
static uint16_t _srv_uuid;


static void ble_evt_callback(Wrapper::BLE::Client::EvtType evt, uint16_t handle) {
	_status = evt;
	_timer_handle.restart();
}

static void scan_callback(Wrapper::BLE::Client::AdvReport report) {
	if (strlen(report.name) <= 0) return;

	NRF_LOG_INFO("scan device :%s", report.name);
	for (int i = 0; i < SCAN_MAX_BUF_NUM; i++) {
		if (strcmp(_scan_dev_list[i].name, report.name) == 0) break;
		if (_scan_dev_list[i].addr[0] == 0) {
			_scan_dev_list[i] = report;
			break;
		}
	}
}

static void db_callback(uint16_t srv_uuid, Wrapper::BLE::Client::CharHandle char_handle[], uint16_t count) {
	_srv_uuid = srv_uuid;
	_status = Wrapper::BLE::Client::EvtType::SERVICE_DISCOVER_EVT;
	size_t cp_size = (count > CHAR_MAX_BUF_NUM ? CHAR_MAX_BUF_NUM : count) * sizeof(Wrapper::BLE::Client::CharHandle);
	memcpy(_char_handle, char_handle, cp_size);
	_timer_handle.restart();
}

static void dongle_task(void *arg) {
	usb_cli::write("\nBLE %s\n", Wrapper::BLE::Client::evt_to_str(_status).data());
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
		usb_cli::write("%s discovered service UUID: %04X\n", _target_device.name, _srv_uuid);
		for (int i = 0; (i < CHAR_MAX_BUF_NUM) && (_char_handle[i].uuid != 0); i++) {
			usb_cli::write("Characteristic UUID: %04X\n", _char_handle[i].uuid);
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
	_srv_uuid = 0;
	memset(_char_handle, 0, sizeof(_char_handle));
	for (int i = 0; (i < SCAN_MAX_BUF_NUM) && (_scan_dev_list[i].addr[0] != 0); i++) {
		if (strcmp(_scan_dev_list[i].name, name.data()) == 0) {
			_target_device = _scan_dev_list[i];
			return Wrapper::BLE::Client::connection(_scan_dev_list[i].addr, timeout);
		}
	}
	return -1;
}

int ble_disconnect() {
	return Wrapper::BLE::Client::disconnection();
}

int init() {
	Wrapper::BLE::Client::register_evt_callback(ble_evt_callback);
	Wrapper::BLE::Client::register_scan_callback(scan_callback);
	Wrapper::BLE::Client::register_db_callback(db_callback);
	usb_cli::enable();
	_timer_handle.create(dongle_task, Wrapper::AppTimer::CALL_IMMEDIATE);
	return 0;
}


}
