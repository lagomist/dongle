#pragma once
#include <string>
#include <cstdint>
#include <stdarg.h>


namespace Utils {

using IBuf = std::basic_string_view<char>;
using OBuf = std::basic_string<char>;

OBuf vsprint(const char format[], va_list args)  __attribute__((__format__ (__printf__, 1, 0)));
OBuf sprint(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));

}
