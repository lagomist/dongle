#pragma once

#include <cstdint>
#include <string_view>

namespace dongle {

void ble_scan(uint16_t timeout);

// 支持设备名和地址连接
int ble_connect(std::string_view name, uint16_t timeout);

int ble_select(uint16_t char_uuid);
int ble_send(std::string_view buf);
int ble_disconnect();
int init();

}
