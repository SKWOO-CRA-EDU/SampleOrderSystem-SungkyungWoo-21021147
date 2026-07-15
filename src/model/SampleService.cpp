#include "SampleService.h"

#include <cmath>

#include "Calculations.h"

namespace model {

SampleService::RegisterResult SampleService::RegisterSample(const std::string& sampleId, const std::string& name,
                                                              double avgProductionTime, int yieldNumerator) {
    if (yieldNumerator < 1 || yieldNumerator > domain::YIELD_DENOMINATOR) {  // 수율 ∈ (0,1] [ADR-Q3][ADR-Q4]
        return {false, RejectionCode::InvalidYield, {}};
    }
    if (avgProductionTime <= 0.0) {  // [ADR-Q12]
        return {false, RejectionCode::InvalidAvgProductionTime, {}};
    }

    domain::SampleRecord sample{sampleId, name, avgProductionTime, yieldNumerator, /*stockQuantity=*/0};
    domain::WriteOutcome outcome = sampleRepo_.Add(sample);
    if (outcome == domain::WriteOutcome::DuplicateKey) {
        return {false, RejectionCode::DuplicateSampleId, {}};
    }
    return {true, RejectionCode::DuplicateSampleId /*미사용*/, sample};
}

std::vector<domain::SampleRecord> SampleService::ListSamples() const {
    return sampleRepo_.FindAll();
}

std::vector<domain::SampleRecord> SampleService::SearchSamples(const std::string& query) const {
    std::vector<domain::SampleRecord> result;
    if (query.empty()) {
        return result;
    }
    bool queryIsNumber = false;
    double queryAsNumber = 0.0;
    try {
        size_t consumed = 0;
        queryAsNumber = std::stod(query, &consumed);
        queryIsNumber = (consumed == query.size());
    } catch (...) {
        queryIsNumber = false;
    }

    for (const auto& sample : sampleRepo_.FindAll()) {
        bool matches = sample.sampleId.find(query) != std::string::npos ||
                        sample.name.find(query) != std::string::npos;
        if (!matches && queryIsNumber) {
            constexpr double kEpsilon = 1e-9;
            double effectiveYield = EffectiveYield(sample.yieldNumerator);
            matches = std::abs(sample.avgProductionTime - queryAsNumber) < kEpsilon ||
                      std::abs(effectiveYield - queryAsNumber) < kEpsilon;
        }
        if (matches) {
            result.push_back(sample);
        }
    }
    return result;
}

SampleService::DeleteResult SampleService::DeleteSample(const std::string& sampleId) {
    if (!orderRepo_.FindBySampleId(sampleId).empty()) {  // 참조무결성(ADR-E8) — Model 책임
        return {false, RejectionCode::SampleReferenced};
    }
    domain::WriteOutcome outcome = sampleRepo_.Delete(sampleId);
    if (outcome == domain::WriteOutcome::NotFound) {
        return {false, RejectionCode::SampleNotFound};
    }
    return {true, RejectionCode::SampleNotFound /*미사용*/};
}

}  // namespace model
