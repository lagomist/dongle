#include "gatt_client.h"
#include "nrf_delay.h"

namespace dongle {

// 设备名称数组  英文名称
static char const m_target_periph_name[] = "NRF_GATT_TEST";
// GATT数据缓存
static uint8_t data_buffer[544] = {0};
// ibeacon data buffer
static char adv_data_buf[200] = {0};


}
