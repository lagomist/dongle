#pragma once

#include "gatt_client.h"
#include <cstdint>
#include <string_view>

namespace dongle {

using ScanMode = Wrapper::BLE::Client::ScanMode;

void ble_scan(uint16_t timeout, ScanMode mode = ScanMode::PHY_1M);

// 支持设备名和地址连接
int ble_connect(std::string_view name, uint16_t timeout);

int ble_select(uint16_t char_uuid);
int ble_send(std::string_view buf);
int ble_disconnect();
int init();

}
