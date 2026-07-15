// docs/CONTRACT.md v4 §2, §5.2, §7 — 저장 방식과 무관한 도메인 자료형.
// Model/View/Controller 어디에도 <iostream>이나 콘솔 I/O를 포함하지 않는다 (CLAUDE.md §2).
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace domain {

// CONTRACT.md §2 — Order.Status 열거형. 저장 문자열 매핑은 OrderStatusCodec(src/infra/persistence)의 책임.
enum class OrderStatus {
    Reserved,
    Rejected,
    Producing,
    Confirmed,
    Release,
};

// CONTRACT.md §5.1/§5.2 — [D] 도메인 거부 범주의 유일한 관용구.
[[nodiscard]] enum class WriteOutcome { Ok, NotFound, DuplicateKey };

// CONTRACT.md §5.2 — [F] 치명 범주의 예외 3종. 저장 방식과 무관하며, 구현체는 반드시 이 3종으로 재포장한다 (ADR-E2).
class StorageUnavailable : public std::runtime_error {
public:
    explicit StorageUnavailable(const std::string& what) : std::runtime_error(what) {}
};

class StorageCorrupted : public std::runtime_error {
public:
    explicit StorageCorrupted(const std::string& what) : std::runtime_error(what) {}
};

class SchemaVersionMismatch : public std::runtime_error {
public:
    explicit SchemaVersionMismatch(const std::string& what) : std::runtime_error(what) {}
};

// CONTRACT.md §7 — 영속 저장 포맷(§3)에 대응하는 저장소 레코드.
struct SampleRecord {
    std::string sampleId;
    std::string name;
    double avgProductionTime;
    int yieldNumerator;
    int64_t stockQuantity;
};

struct OrderRecord {
    int64_t orderId;
    std::string sampleId;
    std::string customerName;
    int64_t orderQuantity;
    OrderStatus status;
    std::string createdAt;
};

// CONTRACT.md §3 — Yield(실효값) = yieldNumerator / YIELD_DENOMINATOR.
constexpr int YIELD_DENOMINATOR = 10000;

}  // namespace domain
