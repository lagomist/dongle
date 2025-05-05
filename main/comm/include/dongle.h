#pragma once

#include <cstdint>
#include <string_view>

namespace dongle {

void ble_scan(uint16_t timeout);
int ble_connect(std::string_view name, uint16_t timeout);
int ble_disconnect();
int init();

}
