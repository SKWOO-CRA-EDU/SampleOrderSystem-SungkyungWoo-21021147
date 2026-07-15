// IClock의 실제(Real) 구현체. 합성 루트(main.cpp)가 주입한다(ADR-H3).
#pragma once

#include "../../ports/IClock.h"

namespace infra::console {

class SystemClock : public ports::IClock {
public:
    std::string NowIso8601Utc() const override;
};

}  // namespace infra::console
