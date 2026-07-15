// 시료등록/조회/검색/삭제. Model — Repository 인터페이스에만 의존한다(CLAUDE.md §2/§4).
#pragma once

#include <string>
#include <vector>

#include "../ports/IOrderRepository.h"
#include "../ports/ISampleRepository.h"
#include "Views.h"

namespace model {

class SampleService {
public:
    SampleService(ports::ISampleRepository& sampleRepo, ports::IOrderRepository& orderRepo)
        : sampleRepo_(sampleRepo), orderRepo_(orderRepo) {}

    struct RegisterResult {
        bool ok;
        RejectionCode code;
        domain::SampleRecord sample;
    };
    // FR-01/02/03 — 새 시료는 재고수량 0으로 시작한다. 수율/평균생산시간 입력 검증(ADR-Q3/Q4/Q12).
    RegisterResult RegisterSample(const std::string& sampleId, const std::string& name,
                                   double avgProductionTime, int yieldNumerator);

    std::vector<domain::SampleRecord> ListSamples() const;                    // FR-04
    std::vector<domain::SampleRecord> SearchSamples(const std::string& query) const;  // FR-05, ADR-Q14

    struct DeleteResult {
        bool ok;
        RejectionCode code;
    };
    // FR-33 — 참조하는 Order가 하나도 없을 때만 성공(ADR-C1/C2/E8).
    DeleteResult DeleteSample(const std::string& sampleId);

private:
    ports::ISampleRepository& sampleRepo_;
    ports::IOrderRepository& orderRepo_;
};

}  // namespace model
