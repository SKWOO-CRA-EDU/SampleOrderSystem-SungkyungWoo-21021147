#include "ConsoleOutputPort.h"

#include <iomanip>
#include <iostream>

namespace infra::console {

namespace {

// CC-11 — 재고/주문 상태 색상 전용. 색이 꺼져도 라벨 텍스트만으로 의미가 통한다.
constexpr const char* kColorReset = "\x1b[0m";
constexpr const char* kColorGreen = "\x1b[32m";
constexpr const char* kColorYellow = "\x1b[33m";
constexpr const char* kColorRed = "\x1b[31m";
constexpr const char* kColorCyan = "\x1b[36m";
constexpr const char* kColorBlue = "\x1b[34m";

const char* StatusLabel(domain::OrderStatus status) {
    switch (status) {
        case domain::OrderStatus::Reserved: return "RESERVED";
        case domain::OrderStatus::Rejected: return "REJECTED";
        case domain::OrderStatus::Producing: return "PRODUCING";
        case domain::OrderStatus::Confirmed: return "CONFIRMED";
        case domain::OrderStatus::Release: return "RELEASE";
    }
    return "?";
}

const char* StatusColor(domain::OrderStatus status) {
    switch (status) {
        case domain::OrderStatus::Reserved: return kColorCyan;
        case domain::OrderStatus::Rejected: return kColorRed;
        case domain::OrderStatus::Producing: return kColorYellow;
        case domain::OrderStatus::Confirmed: return kColorGreen;
        case domain::OrderStatus::Release: return kColorBlue;
    }
    return kColorReset;
}

std::string ColoredStatusLabel(domain::OrderStatus status) {
    return std::string(StatusColor(status)) + StatusLabel(status) + kColorReset;
}

const char* StockStateLabel(model::StockState state) {
    switch (state) {
        case model::StockState::Sufficient: return "여유(Sufficient)";
        case model::StockState::Insufficient: return "부족(Insufficient)";
        case model::StockState::Depleted: return "고갈(Depleted)";
    }
    return "?";
}

const char* StockStateColor(model::StockState state) {
    switch (state) {
        case model::StockState::Sufficient: return kColorGreen;
        case model::StockState::Insufficient: return kColorYellow;
        case model::StockState::Depleted: return kColorRed;
    }
    return kColorReset;
}

std::string ColoredStockStateLabel(model::StockState state) {
    return std::string(StockStateColor(state)) + StockStateLabel(state) + kColorReset;
}

const char* RejectionLabel(model::RejectionCode code) {
    switch (code) {
        case model::RejectionCode::DuplicateSampleId: return "이미 등록된 시료ID입니다";
        case model::RejectionCode::InvalidYield: return "수율은 0 초과 1 이하여야 합니다";
        case model::RejectionCode::InvalidAvgProductionTime: return "평균생산시간은 0보다 커야 합니다";
        case model::RejectionCode::SampleNotFound: return "시료ID를 찾을 수 없습니다";
        case model::RejectionCode::SampleReferenced: return "해당 시료를 참조하는 주문이 있어 삭제할 수 없습니다";
        case model::RejectionCode::InvalidOrderQuantity: return "주문수는 0보다 커야 합니다";
        case model::RejectionCode::OrderNotFound: return "주문을 찾을 수 없습니다";
        case model::RejectionCode::IllegalTransition: return "허용되지 않는 상태 전이입니다";
        case model::RejectionCode::EmptyProductionQueue: return "생산큐가 비어 있습니다";
        case model::RejectionCode::ReleaseNotReady: return "재고가 아직 충분히 채워지지 않아 출고할 수 없습니다";
    }
    return "알 수 없는 오류";
}

void PrintSample(const domain::SampleRecord& sample) {
    std::cout << "  [" << sample.sampleId << "] " << sample.name << " | 평균생산시간=" << sample.avgProductionTime
              << " | 수율=" << (static_cast<double>(sample.yieldNumerator) / domain::YIELD_DENOMINATOR)
              << " | 재고수량=" << sample.stockQuantity << "\n";
}

void PrintOrder(const domain::OrderRecord& order) {
    std::cout << "  #" << order.orderId << " [" << order.sampleId << "] " << order.customerName
              << " | 주문수=" << order.orderQuantity << " | 상태=" << ColoredStatusLabel(order.status)
              << " | 등록=" << order.createdAt << "\n";
}

}  // namespace

void ConsoleOutputPort::ShowSampleRegistered(const domain::SampleRecord& sample) {
    std::cout << "[시료등록 완료]\n";
    PrintSample(sample);
}

void ConsoleOutputPort::ShowSampleList(const std::vector<domain::SampleRecord>& samples) {
    std::cout << "[시료 목록] (" << samples.size() << "건)\n";
    for (const auto& sample : samples) PrintSample(sample);
}

void ConsoleOutputPort::ShowSampleDeleted(const std::string& sampleId) {
    std::cout << "[시료삭제 완료] " << sampleId << "\n";
}

void ConsoleOutputPort::ShowOrderReserved(const domain::OrderRecord& order) {
    std::cout << "[시료예약 완료]\n";
    PrintOrder(order);
}

void ConsoleOutputPort::ShowPendingOrders(const std::vector<domain::OrderRecord>& orders) {
    std::cout << "[접수된 주문목록] (" << orders.size() << "건)\n";
    for (const auto& order : orders) PrintOrder(order);
}

void ConsoleOutputPort::ShowOrderTransitioned(const domain::OrderRecord& order) {
    std::cout << "[주문 상태 변경]\n";
    PrintOrder(order);
}

void ConsoleOutputPort::ShowProductionCompleted(const domain::OrderRecord& order, int64_t actualProductionQuantity,
                                                 int64_t stockReflected) {
    std::cout << "[생산완료 처리]\n";
    PrintOrder(order);
    std::cout << "  실생산량=" << actualProductionQuantity << " | 재고반영량(정상품)=" << stockReflected << "\n";
}

void ConsoleOutputPort::ShowProductionQueue(const std::vector<model::ProductionQueueItemView>& queue) {
    std::cout << "[생산현황] (" << queue.size() << "건, FIFO 순)\n";
    for (const auto& item : queue) {
        std::cout << "  " << item.queuePosition << "순위 ";
        PrintOrder(item.order);
        std::cout << "    부족분=" << item.shortageQuantity << " | 실생산량=" << item.actualProductionQuantity
                  << " | 총생산시간=" << item.totalProductionTime << "\n";
    }
}

void ConsoleOutputPort::ShowOrdersByStatus(domain::OrderStatus status, const std::vector<domain::OrderRecord>& orders) {
    std::cout << "[" << ColoredStatusLabel(status) << "] (" << orders.size() << "건)\n";
    for (const auto& order : orders) PrintOrder(order);
}

void ConsoleOutputPort::ShowStockLevels(const std::vector<model::StockLevelView>& levels) {
    std::cout << "[재고량확인] (" << levels.size() << "건)\n";
    for (const auto& level : levels) {
        std::cout << "  [" << level.sample.sampleId << "] " << level.sample.name
                  << " | 재고수량=" << level.sample.stockQuantity << " | 판단대상주문수=" << level.judgedOrderQuantity
                  << " | 상태=" << ColoredStockStateLabel(level.state) << "\n";
    }
}

void ConsoleOutputPort::ShowError(model::RejectionCode code, const std::string& detail) {
    std::cout << "[오류] " << RejectionLabel(code);
    if (!detail.empty()) std::cout << " (" << detail << ")";
    std::cout << "\n";
}

}  // namespace infra::console
