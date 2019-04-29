#pragma once

#include <sys/time.h>
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

namespace util {

inline uint64_t currentMilliSecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}

} // namespace util
