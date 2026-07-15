// docs/CONTRACT.md v4 §7 — 선언만, 구현 금지. 동결(v3)된 저장소 인터페이스, 한 글자도 바뀌지 않았다.
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../domain/Records.h"

namespace ports {

// 예외(CONTRACT §5.2): StorageUnavailable, StorageCorrupted, SchemaVersionMismatch.
// 아래 모든 메서드는 이 3개 예외를 던질 수 있다. 이 3개 예외 외에는 던지지 않는다.
class ISampleRepository {
public:
    virtual ~ISampleRepository() = default;

    // [R] 정상 결과. 부재는 실패가 아니다.
    virtual std::optional<domain::SampleRecord> FindById(const std::string& sampleId) const = 0;
    virtual std::vector<domain::SampleRecord> FindAll() const = 0;

    // [D] 도메인 거부는 WriteOutcome으로 표현한다.
    virtual domain::WriteOutcome Add(const domain::SampleRecord& sample) = 0;     // 키 중복 시 DuplicateKey
    virtual domain::WriteOutcome Update(const domain::SampleRecord& sample) = 0;  // 대상 없음 시 NotFound
    virtual domain::WriteOutcome Delete(const std::string& sampleId) = 0;         // 대상 없음 시 NotFound.
                                                                                   // 참조무결성은 검증하지 않는다 — 호출 전 확인은 Model 책임(CONTRACT §5.3).
};

}  // namespace ports
