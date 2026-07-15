// ADR-I4 — 외부 테스트 프레임워크를 도입하지 않는다. assert 결과를 직접 집계하는 최소 harness.
#pragma once

#include <string>

namespace harness {

// condition이 false면 실패로 집계하고 description을 표준 출력에 남긴다.
void Check(bool condition, const std::string& description);

// 지금까지의 집계를 출력하고, 실패가 하나라도 있으면 1, 전부 통과하면 0을 반환한다.
int Report();

}  // namespace harness
