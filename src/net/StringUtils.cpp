#include "StringUtils.h"
#include <cstdarg>

std::string StringUtils::Sprintf(const char* inFormat, ...)
{
	va_list args;
	va_start(args, inFormat);
#if _WIN32
	int len = _vscprintf(inFormat, args);
#else
	va_list argsCopy;
	va_copy(argsCopy, args);
	int len = vsnprintf(nullptr, 0, inFormat, args);
	va_end(argsCopy);
#endif

	std::string temp(len + 1, '\0');

#if _WIN32
	_vsnprintf_s(&temp[0], len+1, len, inFormat, args);
#else
	vsnprintf(&temp[0], len, inFormat, args);
#endif
	va_end(args);

	return temp;
}
