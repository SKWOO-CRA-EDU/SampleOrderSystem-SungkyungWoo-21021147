// 계약에 없는 내부 컴포넌트(CONTRACT.md §3 "파일 형식·경로·원자적 교체 구현은 저장소 자유"에 속한다).
// RootDocument(§3) 하나(schemaVersion + samples[] + orders[])를 파일 하나에 담아 관리한다.
// FileSampleRepository/FileOrderRepository가 이 저장소 하나를 공유해 같은 문서를 읽고 쓴다.
// 예외 재포장 경계: 내부에서 발생하는 std::ios_base::failure/std::filesystem::filesystem_error/
// JSON 파싱 예외는 여기서 반드시 domain::StorageUnavailable/StorageCorrupted/SchemaVersionMismatch로 재포장한다 [ADR-E2].
#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../domain/Records.h"

namespace infra::persistence {

class JsonDocumentStore {
public:
    explicit JsonDocumentStore(std::string filePath);

    std::optional<domain::SampleRecord> FindSampleById(const std::string& sampleId) const;
    std::vector<domain::SampleRecord> FindAllSamples() const;
    domain::WriteOutcome AddSample(const domain::SampleRecord& sample);
    domain::WriteOutcome UpdateSample(const domain::SampleRecord& sample);
    domain::WriteOutcome DeleteSample(const std::string& sampleId);

    std::optional<domain::OrderRecord> FindOrderById(int64_t orderId) const;
    std::vector<domain::OrderRecord> FindAllOrders() const;
    std::vector<domain::OrderRecord> FindOrdersByStatus(domain::OrderStatus status) const;
    std::vector<domain::OrderRecord> FindOrdersBySampleId(const std::string& sampleId) const;
    domain::WriteOutcome AddOrder(const domain::OrderRecord& order);
    domain::WriteOutcome UpdateOrder(const domain::OrderRecord& order);

private:
    std::string filePath_;
    mutable bool loaded_ = false;
    mutable std::map<std::string, domain::SampleRecord> samples_;
    mutable std::map<int64_t, domain::OrderRecord> orders_;

    void EnsureLoaded() const;
    void LoadFromDisk() const;
    void SaveAtomic() const;
};

}  // namespace infra::persistence
