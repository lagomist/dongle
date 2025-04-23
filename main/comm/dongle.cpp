#include "gatt_client.h"
#include "usb_cdc.h"
#include "nrf_delay.h"
#define NRF_LOG_MODULE_NAME Dongle
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

namespace dongle {

static void ble_evt_callback(Wrapper::BLE::Client::EvtType evt, uint16_t handle) {
	switch (evt) {
	case Wrapper::BLE::Client::EvtType::SCAN_TIMEOUT_EVT:
		usb_cdc::write("scan timeout");
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

int init() {
	usb_cdc::enable();
	Wrapper::BLE::Client::register_evt_callback(ble_evt_callback);
	Wrapper::BLE::Client::register_scan_callback(scan_callback);
	Wrapper::BLE::Client::scan_start(10000);
	return 0;
}


}
