// 콘솔 출력 추상화. CLAUDE.md §2 — View는 도메인 로직을 갖지 않는다: Model이 계산한 값을
// 표시 형식으로만 변환한다. ADR-H5 — 각 메서드는 구조화된 값(Kind=메서드 자체+인자)만 넘긴다.
// 렌더링된 화면 문자열은 이 포트의 관심사가 아니다. CONTRACT.md 범위 밖(§0 "UI는 범위 밖").
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../domain/Records.h"
#include "../model/Views.h"

namespace ports {

class IOutputPort {
public:
    virtual ~IOutputPort() = default;

    // FR-01 시료등록
    virtual void ShowSampleRegistered(const domain::SampleRecord& sample) = 0;
    // FR-04 시료조회 / FR-05 시료검색
    virtual void ShowSampleList(const std::vector<domain::SampleRecord>& samples) = 0;
    // FR-33 시료삭제
    virtual void ShowSampleDeleted(const std::string& sampleId) = 0;
    // FR-06 시료예약
    virtual void ShowOrderReserved(const domain::OrderRecord& order) = 0;
    // FR-08 접수된 주문목록
    virtual void ShowPendingOrders(const std::vector<domain::OrderRecord>& orders) = 0;
    // FR-09/10 주문승인, FR-11 주문거절, FR-12 CONFIRMED 취소, FR-20 출고 — 전이 결과 공통 표시
    virtual void ShowOrderTransitioned(const domain::OrderRecord& order) = 0;
    // FR-19 생산완료 처리
    virtual void ShowProductionCompleted(const domain::OrderRecord& order,
                                          int64_t actualProductionQuantity,
                                          int64_t stockReflected) = 0;
    // FR-14/15/16 생산라인/생산현황/대기주문
    virtual void ShowProductionQueue(const std::vector<model::ProductionQueueItemView>& queue) = 0;
    // FR-21 주문량확인
    virtual void ShowOrdersByStatus(domain::OrderStatus status,
                                     const std::vector<domain::OrderRecord>& orders) = 0;
    // FR-22 재고량확인
    virtual void ShowStockLevels(const std::vector<model::StockLevelView>& levels) = 0;
    // R5 — 미지원/거부된 요청을 조용히 무시하지 않고 명시적으로 알린다.
    virtual void ShowError(model::RejectionCode code, const std::string& detail) = 0;
};

}  // namespace ports
