// Model이 계산해 Controller/View에 넘기는 표시용 DTO. CONTRACT.md의 영속 스키마에는 대응 필드가 없다
// (§1.4 ProductionQueueItem/재고상태는 계약 밖 파생값) — 여기서 순수 데이터로만 정의한다.
#pragma once

#include <cstdint>
#include <string>

#include "../domain/Records.h"

namespace model {

// PRD FR-22 재고량확인 — Sufficient/Insufficient/Depleted 판정 [ADR-Q5][ADR-Q22].
enum class StockState { Sufficient, Insufficient, Depleted };

struct StockLevelView {
    domain::SampleRecord sample;
    int64_t judgedOrderQuantity;  // 판단 대상 주문의 주문수(활성 주문이 없으면 0)
    StockState state;
};

// CONTRACT.md §1.4 ProductionQueueItem — FR-14/15/16 생산라인/생산현황/대기주문 화면 공용.
struct ProductionQueueItemView {
    domain::OrderRecord order;
    int64_t shortageQuantity;
    int64_t actualProductionQuantity;
    double totalProductionTime;
    int queuePosition;  // 1부터 시작하는 FIFO 순번
};

// Model이 거부한 이유. IOutputPort::ShowError가 화면 문구로 변환한다(View 책임, 여기서는 구조화된 값만).
enum class RejectionCode {
    DuplicateSampleId,
    InvalidYield,
    InvalidAvgProductionTime,
    SampleNotFound,
    SampleReferenced,
    InvalidOrderQuantity,
    OrderNotFound,
    IllegalTransition,
    EmptyProductionQueue,
};

}  // namespace model
