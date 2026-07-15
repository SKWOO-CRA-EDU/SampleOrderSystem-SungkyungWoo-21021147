// 주문 상태전이·재고증감·생산큐 계산. Model — Repository/Clock 인터페이스에만 의존한다(CLAUDE.md §2/§4).
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../ports/IClock.h"
#include "../ports/IOrderRepository.h"
#include "../ports/ISampleRepository.h"
#include "Views.h"

namespace model {

// T2(생산큐 등록) 시점에 확정되는 생산 계획. CONTRACT.md의 OrderRecord 스키마에는 대응 필드가 없어
// Model 내부 휘발성 캐시로만 보관한다(앱 재시작 시 소실 — CompleteProduction/ProductionStatusDisplay가
// 캐시 미스 시 그 시점 현재 재고로 재계산하는 폴백을 쓴다. 재시작 전후 계산값이 달라질 수 있는
// 알려진 한계다).
struct ProductionPlan {
    int64_t shortageQuantity;
    int64_t actualProductionQuantity;
    double totalProductionTime;
};

class OrderService {
public:
    OrderService(ports::ISampleRepository& sampleRepo, ports::IOrderRepository& orderRepo, ports::IClock& clock)
        : sampleRepo_(sampleRepo), orderRepo_(orderRepo), clock_(clock) {}

    struct ReserveResult {
        bool ok;
        RejectionCode code;
        domain::OrderRecord order;
    };
    // FR-06/07 — 등록된 시료ID 참조 검증(ADR-R7), 주문수>0 검증(ADR-Q12), 채번은 Model 책임(ADR-E4/Q15).
    ReserveResult ReserveOrder(const std::string& sampleId, const std::string& customerName, int64_t orderQuantity);

    std::vector<domain::OrderRecord> PendingOrderList() const;  // FR-08

    struct TransitionResult {
        bool ok;
        RejectionCode code;
        domain::OrderRecord order;
    };
    TransitionResult ApproveOrder(int64_t orderId);           // T1/T2 — FR-09/10, ADR-Q9/Q21, R2 단일 원자 메서드
    TransitionResult RejectOrder(int64_t orderId);            // T3 — FR-11
    TransitionResult CancelConfirmedOrder(int64_t orderId);   // T6 — FR-12, ADR-Q7
    TransitionResult ReleaseOrder(int64_t orderId);           // T5 — FR-20, ADR-Q24

    struct CompleteResult {
        bool ok;
        RejectionCode code;
        domain::OrderRecord order;
        int64_t actualProductionQuantity;
        int64_t stockReflected;
    };
    CompleteResult CompleteProduction();  // T4 — FR-19/28, FIFO 큐 선두

    // FR-14/15/16 — 생산라인/생산현황/대기주문 공용. FIFO 순서(등록순=orderId를 진입순서 대리값으로 사용).
    std::vector<ProductionQueueItemView> ProductionStatusDisplay() const;

    // FR-21 — RESERVED/CONFIRMED/PRODUCING/RELEASE 4개 상태(REJECTED 제외), 고정 순서.
    std::vector<std::pair<domain::OrderStatus, std::vector<domain::OrderRecord>>> CheckOrderVolume() const;

    // FR-22 — 시료별 재고상태(ADR-Q5/Q22). 판단 대상 주문 = 해당 시료의 RESERVED 주문 중 가장 오래된 것(없으면 0).
    std::vector<StockLevelView> CheckStockLevel() const;

private:
    ports::ISampleRepository& sampleRepo_;
    ports::IOrderRepository& orderRepo_;
    ports::IClock& clock_;
    std::map<int64_t, ProductionPlan> productionPlans_;

    // R2 — 상태전이와 재고증감을 하나의 논리적 단위로 묶는다. sampleRepo_.Update 성공 후 orderRepo_.Update가
    // 실패(예외/WriteOutcome!=Ok)하면 sample을 원래 값으로 보정 Update해 되돌린다(최선 노력 보상 트랜잭션 —
    // CONTRACT §7이 두 저장소를 아우르는 단일 트랜잭션 메서드를 제공하지 않는 한계 안에서의 최선책).
    bool ApplyOrderTransition(const domain::SampleRecord& originalSample, const domain::SampleRecord& newSample,
                               const domain::OrderRecord& newOrder);

    ProductionPlan GetOrComputePlan(const domain::OrderRecord& order, const domain::SampleRecord& sample) const;
};

}  // namespace model
