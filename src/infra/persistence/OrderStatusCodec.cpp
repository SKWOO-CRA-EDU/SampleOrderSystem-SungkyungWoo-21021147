#include "OrderStatusCodec.h"

#include <stdexcept>

namespace infra::persistence {

domain::OrderStatus DecodeOrderStatus(const std::string& stored) {
    if (stored == "RESERVED") return domain::OrderStatus::Reserved;
    if (stored == "REJECTED") return domain::OrderStatus::Rejected;
    if (stored == "PRODUCING") return domain::OrderStatus::Producing;
    if (stored == "CONFIRMED") return domain::OrderStatus::Confirmed;
    if (stored == "RELEASE") return domain::OrderStatus::Release;
    throw std::runtime_error("OrderStatusCodec: unknown stored status '" + stored + "'");
}

std::string EncodeOrderStatus(domain::OrderStatus status) {
    switch (status) {
        case domain::OrderStatus::Reserved: return "RESERVED";
        case domain::OrderStatus::Rejected: return "REJECTED";
        case domain::OrderStatus::Producing: return "PRODUCING";
        case domain::OrderStatus::Confirmed: return "CONFIRMED";
        case domain::OrderStatus::Release: return "RELEASE";
    }
    throw std::runtime_error("OrderStatusCodec: unknown OrderStatus enum value");
}

}  // namespace infra::persistence
