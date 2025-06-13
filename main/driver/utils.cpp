#include "utils.h"

namespace Utils {


OBuf vsprint(const char format[], va_list args) {
	OBuf out;
	va_list args_copy;
	va_copy(args_copy, args);
	int len = vsnprintf(nullptr, 0, format, args_copy);
	if (len < 0)
		return out;
	out.reserve(len + 1);
	out.resize(len);
	vsnprintf((char*)out.data(), out.capacity(), format, args);
	return out;
}

OBuf sprint(const char format[], ...) {
	va_list args;
	va_start(args, format);
	auto out = vsprint(format, args);
	va_end(args);
	return out;
}

}
