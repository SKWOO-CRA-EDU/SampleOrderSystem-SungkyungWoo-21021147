#include "InMemorySampleRepository.h"

namespace infra {

std::optional<domain::SampleRecord> InMemorySampleRepository::FindById(const std::string& sampleId) const {
    auto it = samples_.find(sampleId);
    if (it == samples_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<domain::SampleRecord> InMemorySampleRepository::FindAll() const {
    std::vector<domain::SampleRecord> result;
    result.reserve(samples_.size());
    for (const auto& [id, sample] : samples_) {
        result.push_back(sample);
    }
    return result;
}

domain::WriteOutcome InMemorySampleRepository::Add(const domain::SampleRecord& sample) {
    if (samples_.contains(sample.sampleId)) {
        return domain::WriteOutcome::DuplicateKey;
    }
    samples_.emplace(sample.sampleId, sample);
    return domain::WriteOutcome::Ok;
}

domain::WriteOutcome InMemorySampleRepository::Update(const domain::SampleRecord& sample) {
    auto it = samples_.find(sample.sampleId);
    if (it == samples_.end()) {
        return domain::WriteOutcome::NotFound;
    }
    it->second = sample;
    return domain::WriteOutcome::Ok;
}

domain::WriteOutcome InMemorySampleRepository::Delete(const std::string& sampleId) {
    auto it = samples_.find(sampleId);
    if (it == samples_.end()) {
        return domain::WriteOutcome::NotFound;
    }
    samples_.erase(it);
    return domain::WriteOutcome::Ok;
}

}  // namespace infra
