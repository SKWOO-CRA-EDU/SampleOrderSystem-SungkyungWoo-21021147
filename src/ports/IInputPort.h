// 콘솔 입력 추상화. CLAUDE.md §2 — Controller는 iostream을 직접 호출하지 않는다.
// 실제 콘솔 입력(src/infra/console)과 테스트용 FakeInputPort(tests/fakes) 양쪽이 구현한다.
// CONTRACT.md 범위 밖(§0 "UI·구현코드는 범위 밖") — 저장소 계약과 무관.
#pragma once

#include <string>

namespace ports {

class IInputPort {
public:
    virtual ~IInputPort() = default;

    // 한 줄을 읽어 반환한다. 입력 스트림이 끝나면 빈 문자열을 반환한다.
    virtual std::string ReadLine() = 0;
};

}  // namespace ports
