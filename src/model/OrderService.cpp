#include "OrderService.h"

#include <algorithm>

#include "Calculations.h"

namespace model {

OrderService::ReserveResult OrderService::ReserveOrder(const std::string& sampleId, const std::string& customerName,
                                                         int64_t orderQuantity) {
    if (orderQuantity <= 0) {  // [ADR-Q12]
        return {false, RejectionCode::InvalidOrderQuantity, {}};
    }
    if (!sampleRepo_.FindById(sampleId)) {  // 참조무결성(ADR-R7) — 등록된 시료ID인지 확인
        return {false, RejectionCode::SampleNotFound, {}};
    }

    // 채번(ADR-E4/Q15) — Repository는 NextOrderId를 제공하지 않는다. Model이 FindAll() 최대값+1을 계산한다.
    int64_t nextId = 1;
    for (const auto& existing : orderRepo_.FindAll()) {
        if (existing.orderId >= nextId) nextId = existing.orderId + 1;
    }

    domain::OrderRecord order{nextId, sampleId, customerName, orderQuantity,
                               domain::OrderStatus::Reserved, clock_.NowIso8601Utc()};
    domain::WriteOutcome outcome = orderRepo_.Add(order);
    // Q10(단일 프로세스 순차 처리) 하에서는 발생하지 않아야 하나, Add의 DuplicateKey가 최종 방어선이다(ADR-E4).
    while (outcome == domain::WriteOutcome::DuplicateKey) {
        order.orderId += 1;
        outcome = orderRepo_.Add(order);
    }
    return {true, RejectionCode::InvalidOrderQuantity /*미사용*/, order};
}

std::vector<domain::OrderRecord> OrderService::PendingOrderList() const {
    return orderRepo_.FindByStatus(domain::OrderStatus::Reserved);
}

bool OrderService::ApplyOrderTransition(const domain::SampleRecord& originalSample,
                                         const domain::SampleRecord& newSample,
                                         const domain::OrderRecord& newOrder) {
    if (sampleRepo_.Update(newSample) != domain::WriteOutcome::Ok) {
        return false;
    }
    try {
        if (orderRepo_.Update(newOrder) != domain::WriteOutcome::Ok) {
            (void)sampleRepo_.Update(originalSample);  // 최선 노력 보상 롤백 [ADR-R2] — 결과를 의도적으로 무시한다
            return false;
        }
    } catch (...) {
        (void)sampleRepo_.Update(originalSample);  // 최선 노력 보상 롤백 후 예외를 그대로 전파 [ADR-R2]
        throw;
    }
    return true;
}

OrderService::TransitionResult OrderService::ApproveOrder(int64_t orderId) {
    auto orderOpt = orderRepo_.FindById(orderId);
    if (!orderOpt) return {false, RejectionCode::OrderNotFound, {}};
    domain::OrderRecord order = *orderOpt;
    if (order.status != domain::OrderStatus::Reserved) {
        return {false, RejectionCode::IllegalTransition, order};
    }

    auto sampleOpt = sampleRepo_.FindById(order.sampleId);
    if (!sampleOpt) return {false, RejectionCode::SampleNotFound, order};
    domain::SampleRecord sample = *sampleOpt;

    int64_t originalStock = sample.stockQuantity;
    bool sufficient = originalStock >= order.orderQuantity;  // T1/T2 분기 조건 [ADR-Q9]

    domain::SampleRecord newSample = sample;
    newSample.stockQuantity = originalStock - order.orderQuantity;  // 두 분기 모두 차감 [ADR-Q9][ADR-Q21]

    domain::OrderRecord newOrder = order;
    newOrder.status = sufficient ? domain::OrderStatus::Confirmed : domain::OrderStatus::Producing;

    if (!sufficient) {
        // T2 시점에 생산 계획을 확정한다 — 이후 다른 주문의 재고 변동에 영향받지 않는다(§1절 검산 시나리오 근거).
        int64_t shortage = ComputeShortageQuantity(order.orderQuantity, originalStock);
        int64_t apq = ComputeActualProductionQuantity(shortage, sample.yieldNumerator);
        double totalTime = ComputeTotalProductionTime(sample.avgProductionTime, apq);
        productionPlans_[orderId] = ProductionPlan{shortage, apq, totalTime};
    }

    if (!ApplyOrderTransition(sample, newSample, newOrder)) {
        productionPlans_.erase(orderId);
        return {false, RejectionCode::IllegalTransition, order};
    }
    return {true, RejectionCode::IllegalTransition /*미사용*/, newOrder};
}

OrderService::TransitionResult OrderService::RejectOrder(int64_t orderId) {
    auto orderOpt = orderRepo_.FindById(orderId);
    if (!orderOpt) return {false, RejectionCode::OrderNotFound, {}};
    domain::OrderRecord order = *orderOpt;
    // R5 — RESERVED 외 모든 상태(PRODUCING/CONFIRMED/REJECTED/RELEASE)는 명시적으로 거부한다.
    if (order.status != domain::OrderStatus::Reserved) {
        return {false, RejectionCode::IllegalTransition, order};
    }
    domain::OrderRecord newOrder = order;
    newOrder.status = domain::OrderStatus::Rejected;  // 재고수량 변경 없음 [PRD §3.3]
    if (orderRepo_.Update(newOrder) != domain::WriteOutcome::Ok) {
        return {false, RejectionCode::OrderNotFound, order};
    }
    return {true, RejectionCode::OrderNotFound /*미사용*/, newOrder};
}

OrderService::TransitionResult OrderService::CancelConfirmedOrder(int64_t orderId) {
    auto orderOpt = orderRepo_.FindById(orderId);
    if (!orderOpt) return {false, RejectionCode::OrderNotFound, {}};
    domain::OrderRecord order = *orderOpt;
    if (order.status != domain::OrderStatus::Confirmed) {
        return {false, RejectionCode::IllegalTransition, order};
    }
    auto sampleOpt = sampleRepo_.FindById(order.sampleId);
    if (!sampleOpt) return {false, RejectionCode::SampleNotFound, order};
    domain::SampleRecord sample = *sampleOpt;

    domain::SampleRecord newSample = sample;
    newSample.stockQuantity = sample.stockQuantity + order.orderQuantity;  // 승인 시 차감분 복구 [ADR-Q7]
    domain::OrderRecord newOrder = order;
    newOrder.status = domain::OrderStatus::Rejected;

    if (!ApplyOrderTransition(sample, newSample, newOrder)) {
        return {false, RejectionCode::IllegalTransition, order};
    }
    return {true, RejectionCode::IllegalTransition /*미사용*/, newOrder};
}

OrderService::TransitionResult OrderService::ReleaseOrder(int64_t orderId) {
    auto orderOpt = orderRepo_.FindById(orderId);
    if (!orderOpt) return {false, RejectionCode::OrderNotFound, {}};
    domain::OrderRecord order = *orderOpt;
    if (order.status != domain::OrderStatus::Confirmed) {
        return {false, RejectionCode::IllegalTransition, order};
    }
    auto sampleOpt = sampleRepo_.FindById(order.sampleId);
    if (!sampleOpt) return {false, RejectionCode::SampleNotFound, order};
    if (sampleOpt->stockQuantity < 0) {  // 출고 게이트: 재고 >= 0 [ADR-Q24]
        return {false, RejectionCode::ReleaseNotReady, order};
    }
    domain::OrderRecord newOrder = order;
    newOrder.status = domain::OrderStatus::Release;  // 재고 변경 없음 — T1/T2에서 이미 차감 완료 [ADR-Q24]
    if (orderRepo_.Update(newOrder) != domain::WriteOutcome::Ok) {
        return {false, RejectionCode::OrderNotFound, order};
    }
    return {true, RejectionCode::OrderNotFound /*미사용*/, newOrder};
}

ProductionPlan OrderService::GetOrComputePlan(const domain::OrderRecord& order,
                                               const domain::SampleRecord& sample) const {
    auto it = productionPlans_.find(order.orderId);
    if (it != productionPlans_.end()) {
        return it->second;
    }
    // 캐시 미스(재시작 등) — 최선 노력으로 현재 재고 기준 재계산한다. 알려진 한계(OrderService.h 주석).
    int64_t shortage = ComputeShortageQuantity(order.orderQuantity, sample.stockQuantity);
    int64_t apq = ComputeActualProductionQuantity(shortage, sample.yieldNumerator);
    double totalTime = ComputeTotalProductionTime(sample.avgProductionTime, apq);
    return ProductionPlan{shortage, apq, totalTime};
}

OrderService::CompleteResult OrderService::CompleteProduction() {
    auto producing = orderRepo_.FindByStatus(domain::OrderStatus::Producing);
    if (producing.empty()) {
        return {false, RejectionCode::EmptyProductionQueue, {}, 0, 0};
    }
    // FIFO(등록순) — CONTRACT에 별도의 "큐 진입 시각" 필드가 없어 orderId를 진입 순서 대리값으로 쓴다.
    auto headIt = std::min_element(producing.begin(), producing.end(),
                                    [](const domain::OrderRecord& a, const domain::OrderRecord& b) {
                                        return a.orderId < b.orderId;
                                    });
    domain::OrderRecord order = *headIt;

    auto sampleOpt = sampleRepo_.FindById(order.sampleId);
    if (!sampleOpt) return {false, RejectionCode::SampleNotFound, order, 0, 0};
    domain::SampleRecord sample = *sampleOpt;

    ProductionPlan plan = GetOrComputePlan(order, sample);
    int64_t stockReflected = ComputeStockReflected(plan.actualProductionQuantity, sample.yieldNumerator);

    domain::SampleRecord newSample = sample;
    newSample.stockQuantity = sample.stockQuantity + stockReflected;  // [ADR-Q13]
    domain::OrderRecord newOrder = order;
    newOrder.status = domain::OrderStatus::Confirmed;

    if (!ApplyOrderTransition(sample, newSample, newOrder)) {
        return {false, RejectionCode::IllegalTransition, order, 0, 0};
    }
    productionPlans_.erase(order.orderId);
    return {true, RejectionCode::IllegalTransition /*미사용*/, newOrder, plan.actualProductionQuantity, stockReflected};
}

std::vector<ProductionQueueItemView> OrderService::ProductionStatusDisplay() const {
    auto producing = orderRepo_.FindByStatus(domain::OrderStatus::Producing);
    std::sort(producing.begin(), producing.end(),
              [](const domain::OrderRecord& a, const domain::OrderRecord& b) { return a.orderId < b.orderId; });

    std::vector<ProductionQueueItemView> result;
    int position = 1;
    for (const auto& order : producing) {
        auto sampleOpt = sampleRepo_.FindById(order.sampleId);
        if (!sampleOpt) continue;  // 참조무결성 위반 상황(정상 흐름에서는 발생하지 않음)
        ProductionPlan plan = GetOrComputePlan(order, *sampleOpt);
        result.push_back(ProductionQueueItemView{order, plan.shortageQuantity, plan.actualProductionQuantity,
                                                   plan.totalProductionTime, position});
        ++position;
    }
    return result;
}

std::vector<std::pair<domain::OrderStatus, std::vector<domain::OrderRecord>>> OrderService::CheckOrderVolume() const {
    static const domain::OrderStatus kStatuses[] = {
        domain::OrderStatus::Reserved,
        domain::OrderStatus::Confirmed,
        domain::OrderStatus::Producing,
        domain::OrderStatus::Release,
    };  // FR-21 — REJECTED 제외
    std::vector<std::pair<domain::OrderStatus, std::vector<domain::OrderRecord>>> result;
    for (domain::OrderStatus status : kStatuses) {
        result.emplace_back(status, orderRepo_.FindByStatus(status));
    }
    return result;
}

std::vector<StockLevelView> OrderService::CheckStockLevel() const {
    std::vector<StockLevelView> result;
    for (const auto& sample : sampleRepo_.FindAll()) {
        int64_t judgedOrderQuantity = 0;
        int64_t oldestReservedId = -1;
        for (const auto& order : orderRepo_.FindBySampleId(sample.sampleId)) {
            if (order.status != domain::OrderStatus::Reserved) continue;
            if (oldestReservedId == -1 || order.orderId < oldestReservedId) {
                oldestReservedId = order.orderId;
                judgedOrderQuantity = order.orderQuantity;
            }
        }
        StockState state = ComputeStockState(sample.stockQuantity, judgedOrderQuantity);
        result.push_back(StockLevelView{sample, judgedOrderQuantity, state});
    }
    return result;
}

}  // namespace model
