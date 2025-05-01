#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string_view>

namespace usb_cli {

int write(const char data[], size_t length);
int write(std::string_view str);

void enable();
int init(); 

}
