#pragma once

#include <uv.h>

#ifdef WIN32
#include <time.h>
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_COPY_MOVE_AND_ASSIGN(TypeName)                                \
    TypeName(const TypeName&) = default;                                      \
    TypeName(TypeName&&) = default;                                           \
    TypeName& operator=(const TypeName&) = default;                           \
    TypeName& operator=(TypeName&&) = default

#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName)                               \
private:                                                                      \
    TypeName(const TypeName&);                                                \
    TypeName& operator=(const TypeName&);                                     \
    TypeName(TypeName&&);                                                     \
    TypeName& operator=(const TypeName&&)

#define CONTAINER_OF(ptr, type, field)                                        \
    ((type*)((char*)(ptr) - ((char*)&((type*)0)->field)))

#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))

#ifdef WIN32
#define bzero(BUF, SIZE) memset((BUF), 0, (SIZE))
#endif

#ifdef WIN32
namespace {
int gettimeofday(struct timeval* tp, void* tzp) {
	(void)tzp;
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = (long)clock;
	tp->tv_usec = (long)wtm.wMilliseconds * 1000;
	return (0);
}
}
#endif

namespace util {

inline uint64_t currentMilliSecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}

} // namespace util
