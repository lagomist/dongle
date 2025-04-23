#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string_view>

namespace usb_cdc {

int write(void *data, size_t length);
int write(std::string_view str);

void process();
void enable();
int init(); 

}