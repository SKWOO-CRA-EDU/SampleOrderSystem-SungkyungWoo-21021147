// CONTRACT.md v4 §7 ISampleRepository의 파일 기반 구현체. 실제 영속 구현체 — JsonDocumentStore에 위임한다.
#pragma once

#include "../../ports/ISampleRepository.h"
#include "JsonDocumentStore.h"

namespace infra::persistence {

class FileSampleRepository : public ports::ISampleRepository {
public:
    explicit FileSampleRepository(JsonDocumentStore& store) : store_(store) {}

    std::optional<domain::SampleRecord> FindById(const std::string& sampleId) const override {
        return store_.FindSampleById(sampleId);
    }
    std::vector<domain::SampleRecord> FindAll() const override { return store_.FindAllSamples(); }
    domain::WriteOutcome Add(const domain::SampleRecord& sample) override { return store_.AddSample(sample); }
    domain::WriteOutcome Update(const domain::SampleRecord& sample) override { return store_.UpdateSample(sample); }
    domain::WriteOutcome Delete(const std::string& sampleId) override { return store_.DeleteSample(sampleId); }

private:
    JsonDocumentStore& store_;
};

}  // namespace infra::persistence
