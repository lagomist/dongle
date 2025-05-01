#include "dongle.h"
#include "gatt_client.h"
#include "usb_cli.h"
#include "timer.h"
#include "nrf_delay.h"
#define NRF_LOG_MODULE_NAME Dongle
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

namespace dongle {

constexpr static uint8_t const SCAN_MAX_NUM = 50;
constexpr static uint8_t const NAME_MAX_LEN = 20;
static Wrapper::AppTimer::Timer _timer_handle;
static Status _status = Status::EMPTY;
static char _scan_buf[SCAN_MAX_NUM][NAME_MAX_LEN];

static void ble_evt_callback(Wrapper::BLE::Client::EvtType evt, uint16_t handle) {
	switch (evt) {
	case Wrapper::BLE::Client::EvtType::SCAN_TIMEOUT_EVT:
		_status = Status::SCAN_TIMEOUT;
		_timer_handle.restart();
		break;
	
	default:
		break;
	}
}

static void scan_callback(const uint8_t *adv_data, uint8_t adv_len) {
	std::string_view adv_name = Wrapper::BLE::Client::get_scan_adv_name(adv_data, adv_len);
	if (adv_name.size() <= 0) return ;

	NRF_LOG_INFO("scan device len %d : %s", adv_name.size(), adv_name.data());
	for (int i = 0; i < SCAN_MAX_NUM; i++) {
		if (strcmp(_scan_buf[i], adv_name.data()) == 0) break;
		if (_scan_buf[i][0] == '\0') {
			strncpy(_scan_buf[i], adv_name.data(), NAME_MAX_LEN - 1);
			break;
		}
	}
}

static void dongle_task(uint32_t arg) {
	switch (_status) {
	case Status::SCAN_TIMEOUT:
		usb_cli::write("\nfound device\n");
		for (int i = 0; (i < SCAN_MAX_NUM) && (_scan_buf[i][0] != '\0'); i++) {
			usb_cli::write(_scan_buf[i], strlen(_scan_buf[i]));
		}
		usb_cli::write("\n");
		break;
	
	default:
		break;
	}
}

void ble_scan(uint16_t timeout) {
	_status = Status::EMPTY;
	memset(_scan_buf, 0, sizeof(_scan_buf));
	Wrapper::BLE::Client::scan_start(timeout);
}

int init() {
	Wrapper::BLE::Client::register_evt_callback(ble_evt_callback);
	Wrapper::BLE::Client::register_scan_callback(scan_callback);
	usb_cli::enable();
	_timer_handle.create(dongle_task, 0);
	return 0;
}


}
