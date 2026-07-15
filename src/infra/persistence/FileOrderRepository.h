// CONTRACT.md v4 §7 IOrderRepository의 파일 기반 구현체. 실제 영속 구현체 — JsonDocumentStore에 위임한다.
#pragma once

#include "../../ports/IOrderRepository.h"
#include "JsonDocumentStore.h"

namespace infra::persistence {

class FileOrderRepository : public ports::IOrderRepository {
public:
    explicit FileOrderRepository(JsonDocumentStore& store) : store_(store) {}

    std::optional<domain::OrderRecord> FindById(int64_t orderId) const override {
        return store_.FindOrderById(orderId);
    }
    std::vector<domain::OrderRecord> FindAll() const override { return store_.FindAllOrders(); }
    std::vector<domain::OrderRecord> FindByStatus(domain::OrderStatus status) const override {
        return store_.FindOrdersByStatus(status);
    }
    std::vector<domain::OrderRecord> FindBySampleId(const std::string& sampleId) const override {
        return store_.FindOrdersBySampleId(sampleId);
    }
    domain::WriteOutcome Add(const domain::OrderRecord& order) override { return store_.AddOrder(order); }
    domain::WriteOutcome Update(const domain::OrderRecord& order) override { return store_.UpdateOrder(order); }

private:
    JsonDocumentStore& store_;
};

}  // namespace infra::persistence
