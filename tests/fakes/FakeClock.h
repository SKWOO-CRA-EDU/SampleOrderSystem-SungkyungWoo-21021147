// ADR-H3 — 테스트용 Fake Clock. 고정된 시각 문자열만 반환한다(실제 sleep 없음).
#pragma once

#include "../../src/ports/IClock.h"

namespace tests {

class FakeClock : public ports::IClock {
public:
    explicit FakeClock(std::string fixed = "2026-07-15T00:00:00Z") : fixed_(std::move(fixed)) {}
    std::string NowIso8601Utc() const override { return fixed_; }

private:
    std::string fixed_;
};

}  // namespace tests
