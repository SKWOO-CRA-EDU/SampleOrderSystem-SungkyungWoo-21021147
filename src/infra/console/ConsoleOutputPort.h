// IOutputPort의 실제(Real) 구현체. Model이 넘긴 구조화된 값을 화면 표시 형식으로만 변환한다
// (CLAUDE.md §2 — View는 도메인 로직을 갖지 않는다). 계산은 전혀 하지 않는다.
#pragma once

#include "../../ports/IOutputPort.h"

namespace infra::console {

class ConsoleOutputPort : public ports::IOutputPort {
public:
    void ShowSampleRegistered(const domain::SampleRecord& sample) override;
    void ShowSampleList(const std::vector<domain::SampleRecord>& samples) override;
    void ShowSampleDeleted(const std::string& sampleId) override;
    void ShowOrderReserved(const domain::OrderRecord& order) override;
    void ShowPendingOrders(const std::vector<domain::OrderRecord>& orders) override;
    void ShowOrderTransitioned(const domain::OrderRecord& order) override;
    void ShowProductionCompleted(const domain::OrderRecord& order, int64_t actualProductionQuantity,
                                  int64_t stockReflected) override;
    void ShowProductionQueue(const std::vector<model::ProductionQueueItemView>& queue) override;
    void ShowOrdersByStatus(domain::OrderStatus status, const std::vector<domain::OrderRecord>& orders) override;
    void ShowStockLevels(const std::vector<model::StockLevelView>& levels) override;
    void ShowError(model::RejectionCode code, const std::string& detail) override;
};

}  // namespace infra::console
