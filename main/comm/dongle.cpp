#include "gatt_client.h"
#include "usb_cli.h"
#include "timer.h"
#include "nrf_delay.h"
#define NRF_LOG_MODULE_NAME Dongle
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

namespace dongle {

static Wrapper::AppTimer::Task _task_handle;

static void ble_evt_callback(Wrapper::BLE::Client::EvtType evt, uint16_t handle) {
	switch (evt) {
	case Wrapper::BLE::Client::EvtType::SCAN_TIMEOUT_EVT:
		usb_cli::write("scan timeout");
		break;
	
	default:
		break;
	}
}

static void scan_callback(const uint8_t *adv_data, uint8_t adv_len) {
	std::string_view adv_name = Wrapper::BLE::Client::get_scan_adv_name(adv_data, adv_len);
	if (adv_name.size() > 0) {
		NRF_LOG_INFO("scan device: %s", adv_name.data());
	}
}

static void dongle_task(uint32_t arg) {
	
}

int init() {
	Wrapper::BLE::Client::register_evt_callback(ble_evt_callback);
	Wrapper::BLE::Client::register_scan_callback(scan_callback);
	usb_cli::enable();
	_task_handle.create(dongle_task, 500);
	return 0;
}


}
