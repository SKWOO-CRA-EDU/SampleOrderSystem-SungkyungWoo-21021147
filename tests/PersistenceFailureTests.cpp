// 축1 명제 #29 — 저장 계층 예외는 6개 어휘로만 표면화된다 [ADR-E2][ADR-H4 Tier2].
// 실제 파일 저장소 구현체(JsonDocumentStore)에 실제 실패(손상/버전 불일치/접근 불가)를 주입한다.
#include <cstdio>
#include <filesystem>
#include <fstream>

#include "../src/infra/persistence/JsonDocumentStore.h"
#include "Harness.h"
#include "TestSuites.h"

namespace tests {

namespace fs = std::filesystem;

namespace {

void WriteRaw(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << content;
}

template <typename ExceptionType, typename Fn>
bool ThrowsExactly(Fn&& fn) {
    try {
        fn();
    } catch (const ExceptionType&) {
        return true;
    } catch (const std::exception&) {
        return false;  // 6개 어휘 밖의 타입이 새어나옴 — 이것 자체가 결함
    }
    return false;  // 아무것도 던지지 않음
}

}  // namespace

void RunPersistenceFailureTests() {
    const std::string malformedJsonPath = "test_scratch_malformed.json";
    const std::string badStatusPath = "test_scratch_badstatus.json";
    const std::string versionMismatchPath = "test_scratch_versionmismatch.json";

    // 손상된 JSON 자체 -> StorageCorrupted (raw JSON 파싱 예외가 새어나오면 안 됨)
    WriteRaw(malformedJsonPath, "{ this is not valid json");
    {
        infra::persistence::JsonDocumentStore store(malformedJsonPath);
        harness::Check(ThrowsExactly<domain::StorageCorrupted>([&] { store.FindAllSamples(); }),
                        "#29 손상된 JSON -> domain::StorageCorrupted (파싱 예외 미노출)");
    }

    // §2의 5개 열거값 밖의 status 문자열 -> StorageCorrupted (ADR-E10 불변식2 위반)
    WriteRaw(badStatusPath,
             R"({"schemaVersion":1,"samples":[],"orders":[{"orderId":1,"sampleId":"S","customerName":"C","orderQuantity":1,"status":"BOGUS","createdAt":"2026-07-15T00:00:00Z"}]})");
    {
        infra::persistence::JsonDocumentStore store(badStatusPath);
        harness::Check(ThrowsExactly<domain::StorageCorrupted>([&] { store.FindAllOrders(); }),
                        "#29 열거값 밖 status('BOGUS') -> domain::StorageCorrupted");
    }

    // schemaVersion 불일치 -> SchemaVersionMismatch (변환하지 않고 종료 — CONTRACT §9 비범위)
    WriteRaw(versionMismatchPath, R"({"schemaVersion":2,"samples":[],"orders":[]})");
    {
        infra::persistence::JsonDocumentStore store(versionMismatchPath);
        harness::Check(ThrowsExactly<domain::SchemaVersionMismatch>([&] { store.FindAllSamples(); }),
                        "#29 schemaVersion 불일치(2 != 1) -> domain::SchemaVersionMismatch");
    }

    // 존재하지 않는 디렉터리로 쓰기 실패 -> StorageUnavailable (std::filesystem::filesystem_error 미노출)
    {
        infra::persistence::JsonDocumentStore store("no_such_dir_xyz/data.json");
        harness::Check(ThrowsExactly<domain::StorageUnavailable>([&] {
                           (void)store.AddSample(domain::SampleRecord{"S", "N", 0.8, 9200, 0});
                       }),
                        "#29 존재하지 않는 디렉터리에 쓰기 -> domain::StorageUnavailable");
    }

    std::remove(malformedJsonPath.c_str());
    std::remove(badStatusPath.c_str());
    std::remove(versionMismatchPath.c_str());
}

}  // namespace tests
