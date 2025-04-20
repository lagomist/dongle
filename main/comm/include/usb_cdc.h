#pragma once

#include <stdint.h>
#include <stddef.h>

namespace usb_cdc {

int write(void *data, size_t length);

void process();
int init(); 

}