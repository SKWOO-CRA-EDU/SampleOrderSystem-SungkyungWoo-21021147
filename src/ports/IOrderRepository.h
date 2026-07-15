// docs/CONTRACT.md v4 §7 — 선언만, 구현 금지. 동결(v3)된 저장소 인터페이스, 한 글자도 바뀌지 않았다.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../domain/Records.h"

namespace ports {

// IOrderRepository에는 의도적으로 Delete/Remove 메서드가 없다 [ADR-C3].
// IOrderRepository에는 의도적으로 NextOrderId 메서드가 없다 [ADR-E4] — 채번은 Model 책임.
//
// 예외(CONTRACT §5.2): StorageUnavailable, StorageCorrupted, SchemaVersionMismatch.
// 아래 모든 메서드는 이 3개 예외를 던질 수 있다. 이 3개 예외 외에는 던지지 않는다.
class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

    // [R] 정상 결과. 부재는 실패가 아니다.
    virtual std::optional<domain::OrderRecord> FindById(int64_t orderId) const = 0;
    virtual std::vector<domain::OrderRecord> FindAll() const = 0;
    virtual std::vector<domain::OrderRecord> FindByStatus(domain::OrderStatus status) const = 0;  // 문자열 아님 [ADR-E6]
    virtual std::vector<domain::OrderRecord> FindBySampleId(const std::string& sampleId) const = 0;  // [ADR-E7]

    // [D] 도메인 거부는 WriteOutcome으로 표현한다.
    virtual domain::WriteOutcome Add(const domain::OrderRecord& order) = 0;     // 키 중복 시 DuplicateKey
    virtual domain::WriteOutcome Update(const domain::OrderRecord& order) = 0;  // 대상 없음 시 NotFound.
                                                                                 // 상태 전이 적법성은 검증하지 않는다 — Model 책임(CONTRACT §5.4).
};

}  // namespace ports
