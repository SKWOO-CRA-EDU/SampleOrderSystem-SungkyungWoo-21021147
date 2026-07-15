// CONTRACT.md v4 §7 IOrderRepository의 인메모리 구현체.
// 실제 저장 매체가 없으므로 StorageUnavailable/StorageCorrupted/SchemaVersionMismatch를 던질 상황 자체가 없다.
#pragma once

#include <map>

#include "../../ports/IOrderRepository.h"

namespace infra {

class InMemoryOrderRepository : public ports::IOrderRepository {
public:
    std::optional<domain::OrderRecord> FindById(int64_t orderId) const override;
    std::vector<domain::OrderRecord> FindAll() const override;
    std::vector<domain::OrderRecord> FindByStatus(domain::OrderStatus status) const override;
    std::vector<domain::OrderRecord> FindBySampleId(const std::string& sampleId) const override;
    domain::WriteOutcome Add(const domain::OrderRecord& order) override;
    domain::WriteOutcome Update(const domain::OrderRecord& order) override;

private:
    std::map<int64_t, domain::OrderRecord> orders_;
};

}  // namespace infra
