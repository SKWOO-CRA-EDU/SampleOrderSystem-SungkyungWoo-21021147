// docs/DECISIONS.md ADR-Q21~Q24 ★ 검산 시나리오 — 재고 30, order1=100, order2=20, 수율 0.92,
// order1 T2 -> order2 T2 -> order1 T4 -> order2 T4 -> 둘 다 출고 -> 최종 재고 0.
#include "../src/infra/repository/InMemoryOrderRepository.h"
#include "../src/infra/repository/InMemorySampleRepository.h"
#include "../src/model/OrderService.h"
#include "../src/model/SampleService.h"
#include "Harness.h"
#include "TestSuites.h"
#include "fakes/FakeClock.h"

namespace tests {

void RunVerificationScenarioTests() {
    infra::InMemorySampleRepository sampleRepo;
    infra::InMemoryOrderRepository orderRepo;
    FakeClock clock;

    harness::Check(sampleRepo.Add(domain::SampleRecord{"V", "Verify", /*avgProductionTime=*/1.0,
                                                        /*yieldNumerator=*/9200, /*stockQuantity=*/30}) ==
                       domain::WriteOutcome::Ok,
                   "검산 setup: 시료 등록(재고 30)");
    model::SampleService sampleService(sampleRepo, orderRepo);
    model::OrderService orderService(sampleRepo, orderRepo, clock);

    auto order1 = orderService.ReserveOrder("V", "Cust1", 100);
    auto order2 = orderService.ReserveOrder("V", "Cust2", 20);
    harness::Check(order1.ok && order2.ok, "검산 setup: order1/order2 예약 성공");

    auto approve1 = orderService.ApproveOrder(order1.order.orderId);
    harness::Check(approve1.ok && approve1.order.status == domain::OrderStatus::Producing,
                    "검산 order1 T2: 재고 30 < 주문 100 -> PRODUCING");
    harness::Check(sampleRepo.FindById("V")->stockQuantity == -70, "검산 order1 T2: 재고 30-100 -> -70");

    auto approve2 = orderService.ApproveOrder(order2.order.orderId);
    harness::Check(approve2.ok && approve2.order.status == domain::OrderStatus::Producing,
                    "검산 order2 T2: 재고 -70 < 주문 20 -> PRODUCING");
    harness::Check(sampleRepo.FindById("V")->stockQuantity == -90, "검산 order2 T2: 재고 -70-20 -> -90");

    auto complete1 = orderService.CompleteProduction();  // FIFO 선두 = order1 (orderId 더 작음)
    harness::Check(complete1.ok && complete1.order.orderId == order1.order.orderId,
                    "검산 order1 T4: FIFO 선두가 order1");
    harness::Check(complete1.actualProductionQuantity == 77, "검산 order1 T4: 실생산량 ceil(70/0.92) -> 77");
    harness::Check(complete1.stockReflected == 70, "검산 order1 T4: 재고반영량 floor(77*0.92) -> 70");
    harness::Check(sampleRepo.FindById("V")->stockQuantity == -20, "검산 order1 T4: 재고 -90+70 -> -20");

    auto complete2 = orderService.CompleteProduction();  // 다음 FIFO 선두 = order2
    harness::Check(complete2.ok && complete2.order.orderId == order2.order.orderId,
                    "검산 order2 T4: 다음 FIFO 선두가 order2");
    harness::Check(complete2.actualProductionQuantity == 22, "검산 order2 T4: 실생산량 ceil(20/0.92) -> 22");
    harness::Check(complete2.stockReflected == 20, "검산 order2 T4: 재고반영량 floor(22*0.92) -> 20");
    harness::Check(sampleRepo.FindById("V")->stockQuantity == 0, "검산 order2 T4: 재고 -20+20 -> 0");

    auto release1 = orderService.ReleaseOrder(order1.order.orderId);
    auto release2 = orderService.ReleaseOrder(order2.order.orderId);
    harness::Check(release1.ok && release1.order.status == domain::OrderStatus::Release,
                    "검산 출고: order1 재고 0>=0 게이트 통과 -> RELEASE");
    harness::Check(release2.ok && release2.order.status == domain::OrderStatus::Release,
                    "검산 출고: order2 재고 0>=0 게이트 통과 -> RELEASE");
    harness::Check(sampleRepo.FindById("V")->stockQuantity == 0,
                    "검산: 최종 재고 0 (30 + 90(생산) - 120(출고) = 0)");
}

}  // namespace tests
