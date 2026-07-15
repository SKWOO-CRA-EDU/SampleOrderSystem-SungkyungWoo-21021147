// ADR-I4 — 외부 테스트 프레임워크 없이 assert-count로 집계하는 harness 진입점.
#include "Harness.h"
#include "TestSuites.h"

int main() {
    tests::RunCalculationTests();
    tests::RunStockTransitionTests();
    tests::RunRepositoryInvariantTests();
    tests::RunPersistenceFailureTests();
    tests::RunVerificationScenarioTests();
    return harness::Report();
}
