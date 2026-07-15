// CONTRACT.md v4 §7 ISampleRepository의 인메모리 구현체.
// 실제 저장 매체가 없으므로 StorageUnavailable/StorageCorrupted/SchemaVersionMismatch를 던질 상황 자체가 없다.
#pragma once

#include <map>

#include "../../ports/ISampleRepository.h"

namespace infra {

class InMemorySampleRepository : public ports::ISampleRepository {
public:
    std::optional<domain::SampleRecord> FindById(const std::string& sampleId) const override;
    std::vector<domain::SampleRecord> FindAll() const override;
    domain::WriteOutcome Add(const domain::SampleRecord& sample) override;
    domain::WriteOutcome Update(const domain::SampleRecord& sample) override;
    domain::WriteOutcome Delete(const std::string& sampleId) override;

private:
    std::map<std::string, domain::SampleRecord> samples_;
};

}  // namespace infra
