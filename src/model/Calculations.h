// CONTRACT.md v4 §8 계산 규칙의 순수 함수 구현. ADR-R4 — 경계값에서 부동소수점 오차가 나지 않도록
// ActualProductionQuantity/재고반영량은 정수 올림/내림 나눗셈으로 계산한다(부족분/수율이 정확히
// 나누어떨어지는 경우에도 IEEE-754 이진 오차로 어긋나지 않는다).
#pragma once

#include <cstdint>

#include "../domain/Records.h"
#include "Views.h"

namespace model {

// Yield(실효값) = yieldNumerator / YIELD_DENOMINATOR [PRD FR-26][ADR-R4]. 표시/총생산시간 계산용.
double EffectiveYield(int yieldNumerator);

// ShortageQuantity = max(0, OrderQuantity - max(0, StockQuantity)) [ADR-Q1][ADR-Q23]
int64_t ComputeShortageQuantity(int64_t orderQuantity, int64_t stockQuantity);

// ActualProductionQuantity = ceil(ShortageQuantity / Yield) — 정수 올림 나눗셈으로 계산 [PRD FR-17][ADR-R4]
int64_t ComputeActualProductionQuantity(int64_t shortageQuantity, int yieldNumerator);

// TotalProductionTime = AvgProductionTime × ActualProductionQuantity [PRD FR-18][ADR-Q16]
double ComputeTotalProductionTime(double avgProductionTime, int64_t actualProductionQuantity);

// 생산완료 시 재고 반영량 = floor(ActualProductionQuantity × Yield) — 정수 내림 나눗셈 [PRD FR-19][ADR-Q13]
int64_t ComputeStockReflected(int64_t actualProductionQuantity, int yieldNumerator);

// 재고 상태 판정 [ADR-Q5][ADR-Q22]: Depleted(<=0) / Insufficient(0<stock<judgedOrderQuantity) / Sufficient(>=judgedOrderQuantity)
StockState ComputeStockState(int64_t stockQuantity, int64_t judgedOrderQuantity);

}  // namespace model
