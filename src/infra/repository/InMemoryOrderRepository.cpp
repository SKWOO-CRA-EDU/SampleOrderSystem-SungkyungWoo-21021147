#include "InMemoryOrderRepository.h"

namespace infra {

std::optional<domain::OrderRecord> InMemoryOrderRepository::FindById(int64_t orderId) const {
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<domain::OrderRecord> InMemoryOrderRepository::FindAll() const {
    std::vector<domain::OrderRecord> result;
    result.reserve(orders_.size());
    for (const auto& [id, order] : orders_) {
        result.push_back(order);
    }
    return result;
}

std::vector<domain::OrderRecord> InMemoryOrderRepository::FindByStatus(domain::OrderStatus status) const {
    std::vector<domain::OrderRecord> result;
    for (const auto& [id, order] : orders_) {
        if (order.status == status) {
            result.push_back(order);
        }
    }
    return result;
}

std::vector<domain::OrderRecord> InMemoryOrderRepository::FindBySampleId(const std::string& sampleId) const {
    std::vector<domain::OrderRecord> result;
    for (const auto& [id, order] : orders_) {
        if (order.sampleId == sampleId) {
            result.push_back(order);
        }
    }
    return result;
}

domain::WriteOutcome InMemoryOrderRepository::Add(const domain::OrderRecord& order) {
    if (orders_.contains(order.orderId)) {
        return domain::WriteOutcome::DuplicateKey;
    }
    orders_.emplace(order.orderId, order);
    return domain::WriteOutcome::Ok;
}

domain::WriteOutcome InMemoryOrderRepository::Update(const domain::OrderRecord& order) {
    auto it = orders_.find(order.orderId);
    if (it == orders_.end()) {
        return domain::WriteOutcome::NotFound;
    }
    it->second = order;
    return domain::WriteOutcome::Ok;
}

}  // namespace infra
