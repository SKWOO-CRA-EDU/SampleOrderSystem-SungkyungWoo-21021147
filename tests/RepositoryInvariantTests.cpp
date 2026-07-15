// 축1 명제 #17/#18 채번·중복, #24/#25 CONTRACT.md v3 §6 불변식 2개.
#include "../src/infra/persistence/OrderStatusCodec.h"
#include "../src/infra/repository/InMemoryOrderRepository.h"
#include "../src/infra/repository/InMemorySampleRepository.h"
#include "../src/model/OrderService.h"
#include "../src/model/SampleService.h"
#include "Harness.h"
#include "TestSuites.h"
#include "fakes/FakeClock.h"

namespace tests {

void RunRepositoryInvariantTests() {
    // #17 OrderId는 자동증가 정수이며 Order의 유일한 키 [ADR-Q15][ADR-E5][ADR-E4]
    {
        infra::InMemorySampleRepository sampleRepo;
        infra::InMemoryOrderRepository orderRepo;
        FakeClock clock;
        harness::Check(sampleRepo.Add(domain::SampleRecord{"S", "Sample", 0.8, 9200, 1000}) == domain::WriteOutcome::Ok,
                       "setup: 시료 등록");
        model::OrderService orderService(sampleRepo, orderRepo, clock);

        auto r1 = orderService.ReserveOrder("S", "Cust", 1);
        auto r2 = orderService.ReserveOrder("S", "Cust", 1);
        auto r3 = orderService.ReserveOrder("S", "Cust", 1);
        harness::Check(r1.order.orderId == 1 && r2.order.orderId == 2 && r3.order.orderId == 3,
                        "#17 OrderId 자동증가: 1,2,3 순서로 부여");

        domain::OrderRecord dup = r1.order;
        harness::Check(orderRepo.Add(dup) == domain::WriteOutcome::DuplicateKey,
                        "#17 Order의 유일한 키: 같은 orderId Add -> DuplicateKey");
    }

    // #18 SampleId는 고유(등록 시 중복 거부) [PRD FR-01][ADR-C1]
    {
        infra::InMemorySampleRepository sampleRepo;
        infra::InMemoryOrderRepository orderRepo;
        model::SampleService sampleService(sampleRepo, orderRepo);
        auto first = sampleService.RegisterSample("DUP", "First", 0.8, 9200);
        harness::Check(first.ok, "#18 setup: 최초 등록 성공");
        auto second = sampleService.RegisterSample("DUP", "Second", 0.5, 5000);
        harness::Check(!second.ok && second.code == model::RejectionCode::DuplicateSampleId,
                        "#18 동일 SampleId 재등록 -> DuplicateSampleId 거부");
        harness::Check(sampleRepo.FindById("DUP")->name == "First",
                        "#18 거부 후에도 기존 시료 데이터 불변");
    }

    // #24 [v3 §6 불변식1] 모든 OrderStatus 값에 대한 FindByStatus 결과 크기의 합 = FindAll 결과 크기 [ADR-E10]
    {
        infra::InMemorySampleRepository sampleRepo;
        infra::InMemoryOrderRepository orderRepo;
        FakeClock clock;
        harness::Check(sampleRepo.Add(domain::SampleRecord{"S", "Sample", 0.8, 9200, 1000}) == domain::WriteOutcome::Ok,
                       "setup: 시료 등록");
        model::OrderService orderService(sampleRepo, orderRepo, clock);

        auto a = orderService.ReserveOrder("S", "Cust", 1);   // RESERVED
        auto b = orderService.ReserveOrder("S", "Cust", 1);
        orderService.ApproveOrder(b.order.orderId);           // T1 -> CONFIRMED (재고충분)
        auto c = orderService.ReserveOrder("S", "Cust", 1);
        orderService.RejectOrder(c.order.orderId);            // REJECTED
        (void)a;

        size_t sumByStatus = orderRepo.FindByStatus(domain::OrderStatus::Reserved).size() +
                              orderRepo.FindByStatus(domain::OrderStatus::Rejected).size() +
                              orderRepo.FindByStatus(domain::OrderStatus::Producing).size() +
                              orderRepo.FindByStatus(domain::OrderStatus::Confirmed).size() +
                              orderRepo.FindByStatus(domain::OrderStatus::Release).size();
        harness::Check(sumByStatus == orderRepo.FindAll().size(),
                        "#24 5개 OrderStatus FindByStatus 합계 == FindAll 크기");
    }

    // #25 저장된 Order.status 값은 §2의 5개 열거값 중 하나 [ADR-E10]
    // — domain::OrderStatus는 C++ enum class이므로 열거값 외 값은 타입 시스템상 표현 자체가 불가능하다
    //   (HARNESS-SURVEY.md 축3 #25 "컴파일 타임 강제"). 영속 계층(문자열 wire format)에서의 위반은
    //   OrderStatusCodec::DecodeOrderStatus가 std::runtime_error를 던지는 것으로 관측한다
    //   (JsonDocumentStore가 이를 domain::StorageCorrupted로 재포장 — Tier2에서 별도 검증).
    {
        bool threw = false;
        try {
            infra::persistence::DecodeOrderStatus("UNKNOWN_STATUS");
        } catch (const std::exception&) {
            threw = true;
        }
        harness::Check(threw, "#25 열거값 밖 저장 문자열('UNKNOWN_STATUS') -> 디코딩 실패(예외)");

        harness::Check(infra::persistence::DecodeOrderStatus("RESERVED") == domain::OrderStatus::Reserved &&
                            infra::persistence::DecodeOrderStatus("REJECTED") == domain::OrderStatus::Rejected &&
                            infra::persistence::DecodeOrderStatus("PRODUCING") == domain::OrderStatus::Producing &&
                            infra::persistence::DecodeOrderStatus("CONFIRMED") == domain::OrderStatus::Confirmed &&
                            infra::persistence::DecodeOrderStatus("RELEASE") == domain::OrderStatus::Release,
                        "#25 5개 열거값 문자열은 모두 정상 디코딩된다");
    }
}

}  // namespace tests
