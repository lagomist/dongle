#pragma once

#include <cstdint>

namespace dongle {

enum Status : uint8_t {
	EMPTY,
	SCAN_TIMEOUT,
	CONNECTED,
	DISCONNECTED
};

void ble_scan(uint16_t timeout);
int init();

}
