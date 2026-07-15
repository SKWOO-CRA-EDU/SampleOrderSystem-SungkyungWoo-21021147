// 축1 명제 #1~5 — 계산 수식(CONTRACT.md v4 §8).
#include "../src/model/Calculations.h"
#include "Harness.h"
#include "TestSuites.h"

namespace tests {

void RunCalculationTests() {
    // #1 Yield(실효값) = yieldNumerator / YIELD_DENOMINATOR [ADR-R4]
    harness::Check(model::EffectiveYield(9200) == 0.92, "#1 Yield = yieldNumerator/YIELD_DENOMINATOR (9200 -> 0.92)");
    harness::Check(model::EffectiveYield(10000) == 1.0, "#1 Yield 상한 경계 (10000 -> 1.0)");

    // #2 ShortageQuantity = max(0, OrderQuantity - max(0, StockQuantity)) [ADR-Q1][ADR-Q23]
    harness::Check(model::ComputeShortageQuantity(170, 0) == 170, "#2 Shortage — 재고 0, 주문 170 -> 170");
    harness::Check(model::ComputeShortageQuantity(30, 100) == 0, "#2 Shortage — 재고 충분(100>=30) -> 0");
    harness::Check(model::ComputeShortageQuantity(100, 30) == 70, "#2 Shortage — §1 검산 order1 (100,30) -> 70");
    harness::Check(model::ComputeShortageQuantity(20, -70) == 20,
                    "#2 Shortage — Q23 클램프: 재고 음수(-70)는 0으로 취급 (20,-70) -> 20 (100이면 오류)");

    // #3 ActualProductionQuantity = ceil(ShortageQuantity / Yield) — ADR-R4 경계값(90/0.9 -> 100 반드시)
    harness::Check(model::ComputeActualProductionQuantity(90, 9000) == 100,
                    "#3 R4 경계값: 부족분 90 / 수율 0.9 -> 정확히 100 (부동소수점 오차 없이)");
    harness::Check(model::ComputeActualProductionQuantity(170, 9200) == 185,
                    "#3 SPEC §4 예시: 부족분 170 / 수율 0.92 -> 185");
    harness::Check(model::ComputeActualProductionQuantity(0, 9200) == 0, "#3 부족분 0 -> 실생산량 0");
    harness::Check(model::ComputeActualProductionQuantity(70, 9200) == 77, "#3 §1 검산 order1: 70/0.92 -> 77");
    harness::Check(model::ComputeActualProductionQuantity(20, 9200) == 22, "#3 §1 검산 order2: 20/0.92 -> 22");

    // #4 TotalProductionTime = AvgProductionTime × ActualProductionQuantity [ADR-Q16]
    harness::Check(model::ComputeTotalProductionTime(0.8, 185) == 148.0, "#4 SPEC §4 예시: 0.8 × 185 -> 148");
    harness::Check(model::ComputeTotalProductionTime(0.92, 77) > 70.8 && model::ComputeTotalProductionTime(0.92, 77) < 70.9,
                    "#4 §1 검산 order1 총생산시간 ≈ 70.84");

    // #5 생산완료 시 재고반영량 = floor(ActualProductionQuantity × Yield) [ADR-Q13]
    harness::Check(model::ComputeStockReflected(185, 9200) == 170, "#5 floor(185×0.92) -> 170 (SPEC §4 정합)");
    harness::Check(model::ComputeStockReflected(100, 9000) == 90, "#5 R4 경계값: floor(100×0.9) -> 정확히 90");
    harness::Check(model::ComputeStockReflected(77, 9200) == 70, "#5 §1 검산 order1: floor(77×0.92) -> 70");
    harness::Check(model::ComputeStockReflected(22, 9200) == 20, "#5 §1 검산 order2: floor(22×0.92) -> 20");
}

}  // namespace tests
