#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string_view>

namespace usb_cli {

int write(std::string_view str);
int write(const char *p_fmt, ...);
int option_printf(const char format[], ...);

void enable();
int init(); 

}
