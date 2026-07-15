#include "Calculations.h"

namespace model {

double EffectiveYield(int yieldNumerator) {
    return static_cast<double>(yieldNumerator) / static_cast<double>(domain::YIELD_DENOMINATOR);
}

int64_t ComputeShortageQuantity(int64_t orderQuantity, int64_t stockQuantity) {
    int64_t availableStock = stockQuantity > 0 ? stockQuantity : 0;
    int64_t shortage = orderQuantity - availableStock;
    return shortage > 0 ? shortage : 0;
}

int64_t ComputeActualProductionQuantity(int64_t shortageQuantity, int yieldNumerator) {
    if (shortageQuantity <= 0) {
        return 0;
    }
    // ceil(shortageQuantity / (yieldNumerator/YIELD_DENOMINATOR))
    //   = ceil(shortageQuantity * YIELD_DENOMINATOR / yieldNumerator)
    //   = (shortageQuantity * YIELD_DENOMINATOR + yieldNumerator - 1) / yieldNumerator  (정수 올림 나눗셈)
    int64_t numerator = shortageQuantity * static_cast<int64_t>(domain::YIELD_DENOMINATOR);
    return (numerator + yieldNumerator - 1) / yieldNumerator;
}

double ComputeTotalProductionTime(double avgProductionTime, int64_t actualProductionQuantity) {
    return avgProductionTime * static_cast<double>(actualProductionQuantity);
}

int64_t ComputeStockReflected(int64_t actualProductionQuantity, int yieldNumerator) {
    // floor(actualProductionQuantity * yieldNumerator / YIELD_DENOMINATOR) — 정수 나눗셈은 음이 아닌 값에서 floor와 동일.
    return (actualProductionQuantity * static_cast<int64_t>(yieldNumerator)) / domain::YIELD_DENOMINATOR;
}

StockState ComputeStockState(int64_t stockQuantity, int64_t judgedOrderQuantity) {
    if (stockQuantity <= 0) {
        return StockState::Depleted;
    }
    if (stockQuantity < judgedOrderQuantity) {
        return StockState::Insufficient;
    }
    return StockState::Sufficient;
}

}  // namespace model
