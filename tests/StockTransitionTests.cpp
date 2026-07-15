// 축1 명제 #6/#7/#9 T1/T2/T6 재고, #10~12 재고상태 판정.
#include "../src/infra/repository/InMemoryOrderRepository.h"
#include "../src/infra/repository/InMemorySampleRepository.h"
#include "../src/model/Calculations.h"
#include "../src/model/OrderService.h"
#include "../src/model/SampleService.h"
#include "Harness.h"
#include "TestSuites.h"
#include "fakes/FakeClock.h"

namespace tests {

namespace {
domain::SampleRecord MakeSample(const std::string& id, int64_t stock, int yieldNumerator = 9200,
                                 double avgProductionTime = 0.8) {
    return domain::SampleRecord{id, "Sample-" + id, avgProductionTime, yieldNumerator, stock};
}
}  // namespace

void RunStockTransitionTests() {
    // #6 T1(RESERVED->CONFIRMED) 시 StockQuantity -= OrderQuantity [ADR-Q9]
    {
        infra::InMemorySampleRepository sampleRepo;
        infra::InMemoryOrderRepository orderRepo;
        FakeClock clock;
        harness::Check(sampleRepo.Add(MakeSample("A", 200)) == domain::WriteOutcome::Ok, "#6 setup: 시료 등록");
        model::SampleService sampleService(sampleRepo, orderRepo);
        model::OrderService orderService(sampleRepo, orderRepo, clock);

        auto reserve = orderService.ReserveOrder("A", "Cust", 100);
        harness::Check(reserve.ok, "#6 setup: 예약 성공");
        auto approve = orderService.ApproveOrder(reserve.order.orderId);
        harness::Check(approve.ok && approve.order.status == domain::OrderStatus::Confirmed,
                        "#6 T1: 재고충분(200>=100) -> CONFIRMED");
        harness::Check(sampleRepo.FindById("A")->stockQuantity == 100, "#6 T1: 재고 200-100 -> 100");
    }

    // #7 T2(RESERVED->PRODUCING) 시 StockQuantity -= OrderQuantity — Q21대로 음수 결과를 단언한다
    {
        infra::InMemorySampleRepository sampleRepo;
        infra::InMemoryOrderRepository orderRepo;
        FakeClock clock;
        harness::Check(sampleRepo.Add(MakeSample("B", 30)) == domain::WriteOutcome::Ok, "#7 setup: 시료 등록");
        model::OrderService orderService(sampleRepo, orderRepo, clock);

        auto reserve = orderService.ReserveOrder("B", "Cust", 100);
        auto approve = orderService.ApproveOrder(reserve.order.orderId);
        harness::Check(approve.ok && approve.order.status == domain::OrderStatus::Producing,
                        "#7 T2: 재고부족(30<100) -> PRODUCING");
        harness::Check(sampleRepo.FindById("B")->stockQuantity == -70,
                        "#7 T2: 재고 30-100 -> -70 (ADR-Q21 음수 허용, 오류 아님)");
    }

    // #9 T6(CONFIRMED->REJECTED) 시 StockQuantity += OrderQuantity [ADR-Q7]
    {
        infra::InMemorySampleRepository sampleRepo;
        infra::InMemoryOrderRepository orderRepo;
        FakeClock clock;
        harness::Check(sampleRepo.Add(MakeSample("C", 200)) == domain::WriteOutcome::Ok, "#9 setup: 시료 등록");
        model::OrderService orderService(sampleRepo, orderRepo, clock);

        auto reserve = orderService.ReserveOrder("C", "Cust", 100);
        auto approveSetup = orderService.ApproveOrder(reserve.order.orderId);  // T1, 재고 200->100
        harness::Check(approveSetup.ok, "#9 setup: T1 승인 성공");
        harness::Check(sampleRepo.FindById("C")->stockQuantity == 100, "#9 setup: T1 이후 재고 100");
        auto cancel = orderService.CancelConfirmedOrder(reserve.order.orderId);
        harness::Check(cancel.ok && cancel.order.status == domain::OrderStatus::Rejected,
                        "#9 T6: CONFIRMED -> REJECTED");
        harness::Check(sampleRepo.FindById("C")->stockQuantity == 200, "#9 T6: 재고 100+100 -> 200 복구");
    }

    // #10~12 재고상태 판정 [ADR-Q5][ADR-Q22] — Depleted 경계는 <=0으로 확장됨
    harness::Check(model::ComputeStockState(0, 50) == model::StockState::Depleted, "#10 재고=0 -> Depleted");
    harness::Check(model::ComputeStockState(-70, 50) == model::StockState::Depleted,
                    "#10 Q22: 재고<0(-70) -> Depleted (경계 확장)");
    harness::Check(model::ComputeStockState(30, 100) == model::StockState::Insufficient,
                    "#11 0<재고(30)<주문수(100) -> Insufficient");
    harness::Check(model::ComputeStockState(100, 100) == model::StockState::Sufficient,
                    "#12 재고(100)>=주문수(100) -> Sufficient (경계 포함)");
    harness::Check(model::ComputeStockState(200, 100) == model::StockState::Sufficient,
                    "#12 재고(200)>=주문수(100) -> Sufficient");
}

}  // namespace tests
