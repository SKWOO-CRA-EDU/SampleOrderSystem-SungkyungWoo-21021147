// ADR-H3 — Model이 시각 인터페이스를 쓰고, 합성 루트가 Real/Fake 구현체를 주입한다.
// CONTRACT.md 밖의 개념(§0 "UI·구현코드는 범위 밖"과 동일한 이유로 계약에 넣지 않는다).
// 실제 sleep은 금지한다(H3) — Model은 이 인터페이스로만 "현재 시각"을 얻는다.
#pragma once

#include <string>

namespace ports {

class IClock {
public:
    virtual ~IClock() = default;

    // CONTRACT.md §3 createdAt 형식(ISO 8601 UTC, 예: "2026-07-15T10:30:00Z")과 동일한 문자열을 반환한다.
    virtual std::string NowIso8601Utc() const = 0;
};

}  // namespace ports
