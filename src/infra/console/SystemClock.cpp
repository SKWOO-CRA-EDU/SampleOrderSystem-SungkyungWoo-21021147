#include "SystemClock.h"

#include <chrono>
#include <cstdio>
#include <ctime>

namespace infra::console {

std::string SystemClock::NowIso8601Utc() const {
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
    gmtime_s(&utc, &now);  // MSVC 스레드 안전 변형
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02dZ", utc.tm_year + 1900, utc.tm_mon + 1,
                  utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec);
    return std::string(buf);
}

}  // namespace infra::console
