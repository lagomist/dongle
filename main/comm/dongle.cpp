#include "gatt_client.h"
#include "nrf_delay.h"
#include "nrf_log.h"

namespace dongle {

// 设备名称数组  英文名称
static char const m_target_periph_name[] = "NRF_GATT_TEST";
// GATT数据缓存
static uint8_t data_buffer[544] = {0};
// ibeacon data buffer
static char adv_data_buf[200] = {0};

static void ble_evt_callback(Wrapper::BLE::Client::EvtType evt, uint16_t handle) {
	switch (evt) {
	case Wrapper::BLE::Client::EvtType::DISCONNECTED_EVT:
		/* code */
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
	Wrapper::BLE::Client::register_evt_callback(ble_evt_callback);
	Wrapper::BLE::Client::register_scan_callback(scan_callback);
	Wrapper::BLE::Client::scan_start(5000);
	return 0;
}


}
