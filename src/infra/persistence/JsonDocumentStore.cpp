#include "JsonDocumentStore.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "MiniJson.h"
#include "OrderStatusCodec.h"

namespace infra::persistence {

namespace fs = std::filesystem;
using json::JsonValue;

namespace {
constexpr int kSchemaVersion = 1;

std::string ReadWholeFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw domain::StorageUnavailable("cannot open file for read: " + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    if (in.bad()) {
        throw domain::StorageUnavailable("read failure: " + path);
    }
    return ss.str();
}

void WriteWholeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw domain::StorageUnavailable("cannot open file for write: " + path);
    }
    out << content;
    out.flush();
    if (!out) {
        throw domain::StorageUnavailable("write failure: " + path);
    }
}

domain::SampleRecord DecodeSample(const JsonValue& v) {
    try {
        const JsonValue* sampleId = v.Find("sampleId");
        const JsonValue* name = v.Find("name");
        const JsonValue* avgProductionTime = v.Find("avgProductionTime");
        const JsonValue* yieldNumerator = v.Find("yieldNumerator");
        const JsonValue* stockQuantity = v.Find("stockQuantity");
        if (!sampleId || !name || !avgProductionTime || !yieldNumerator || !stockQuantity) {
            throw std::runtime_error("SampleRecord: missing field");
        }
        domain::SampleRecord rec;
        rec.sampleId = sampleId->AsString();
        rec.name = name->AsString();
        rec.avgProductionTime = avgProductionTime->AsNumber();
        rec.yieldNumerator = static_cast<int>(yieldNumerator->AsNumber());
        rec.stockQuantity = static_cast<int64_t>(stockQuantity->AsNumber());
        return rec;
    } catch (const std::exception& e) {
        throw domain::StorageCorrupted(std::string("malformed SampleRecord: ") + e.what());
    }
}

JsonValue EncodeSample(const domain::SampleRecord& rec) {
    JsonValue v = JsonValue::MakeObject();
    v.Set("sampleId", JsonValue::MakeString(rec.sampleId));
    v.Set("name", JsonValue::MakeString(rec.name));
    v.Set("avgProductionTime", JsonValue::MakeNumber(rec.avgProductionTime));
    v.Set("yieldNumerator", JsonValue::MakeNumber(rec.yieldNumerator));
    v.Set("stockQuantity", JsonValue::MakeNumber(static_cast<double>(rec.stockQuantity)));
    return v;
}

domain::OrderRecord DecodeOrder(const JsonValue& v) {
    try {
        const JsonValue* orderId = v.Find("orderId");
        const JsonValue* sampleId = v.Find("sampleId");
        const JsonValue* customerName = v.Find("customerName");
        const JsonValue* orderQuantity = v.Find("orderQuantity");
        const JsonValue* status = v.Find("status");
        const JsonValue* createdAt = v.Find("createdAt");
        if (!orderId || !sampleId || !customerName || !orderQuantity || !status || !createdAt) {
            throw std::runtime_error("OrderRecord: missing field");
        }
        domain::OrderRecord rec;
        rec.orderId = static_cast<int64_t>(orderId->AsNumber());
        rec.sampleId = sampleId->AsString();
        rec.customerName = customerName->AsString();
        rec.orderQuantity = static_cast<int64_t>(orderQuantity->AsNumber());
        rec.status = DecodeOrderStatus(status->AsString());  // 5개 열거값 밖이면 std::runtime_error [ADR-E10]
        rec.createdAt = createdAt->AsString();
        return rec;
    } catch (const std::exception& e) {
        throw domain::StorageCorrupted(std::string("malformed OrderRecord: ") + e.what());
    }
}

JsonValue EncodeOrder(const domain::OrderRecord& rec) {
    JsonValue v = JsonValue::MakeObject();
    v.Set("orderId", JsonValue::MakeNumber(static_cast<double>(rec.orderId)));
    v.Set("sampleId", JsonValue::MakeString(rec.sampleId));
    v.Set("customerName", JsonValue::MakeString(rec.customerName));
    v.Set("orderQuantity", JsonValue::MakeNumber(static_cast<double>(rec.orderQuantity)));
    v.Set("status", JsonValue::MakeString(EncodeOrderStatus(rec.status)));
    v.Set("createdAt", JsonValue::MakeString(rec.createdAt));
    return v;
}

}  // namespace

JsonDocumentStore::JsonDocumentStore(std::string filePath) : filePath_(std::move(filePath)) {}

void JsonDocumentStore::EnsureLoaded() const {
    if (!loaded_) {
        LoadFromDisk();
        loaded_ = true;
    }
}

void JsonDocumentStore::LoadFromDisk() const {
    samples_.clear();
    orders_.clear();

    std::error_code ec;
    if (!fs::exists(filePath_, ec)) {
        // CONTRACT §9 비범위 밖의 정상 케이스: 첫 실행이라 파일이 아직 없음(FR-23) — 빈 문서로 시작한다.
        return;
    }

    std::string content = ReadWholeFile(filePath_);

    JsonValue doc;
    try {
        doc = JsonValue::Parse(content);
        if (doc.GetType() != JsonValue::Type::Object) {
            throw std::runtime_error("root is not an object");
        }
    } catch (const std::exception& e) {
        throw domain::StorageCorrupted(std::string("malformed JSON document: ") + e.what());
    }

    const JsonValue* schemaVersion = doc.Find("schemaVersion");
    if (!schemaVersion || schemaVersion->GetType() != JsonValue::Type::Number) {
        throw domain::StorageCorrupted("missing or malformed schemaVersion");
    }
    int storedVersion = static_cast<int>(schemaVersion->AsNumber());
    if (storedVersion != kSchemaVersion) {
        throw domain::SchemaVersionMismatch(
            "stored schemaVersion " + std::to_string(storedVersion) + " != " + std::to_string(kSchemaVersion));
    }

    const JsonValue* samplesArray = doc.Find("samples");
    const JsonValue* ordersArray = doc.Find("orders");
    if (!samplesArray || samplesArray->GetType() != JsonValue::Type::Array ||
        !ordersArray || ordersArray->GetType() != JsonValue::Type::Array) {
        throw domain::StorageCorrupted("missing or malformed samples/orders array");
    }

    for (const auto& sv : samplesArray->AsArray()) {
        domain::SampleRecord rec = DecodeSample(sv);
        if (samples_.contains(rec.sampleId)) {
            throw domain::StorageCorrupted("duplicate sampleId in stored document: " + rec.sampleId);
        }
        samples_.emplace(rec.sampleId, std::move(rec));
    }
    for (const auto& ov : ordersArray->AsArray()) {
        domain::OrderRecord rec = DecodeOrder(ov);
        if (orders_.contains(rec.orderId)) {
            throw domain::StorageCorrupted("duplicate orderId in stored document: " + std::to_string(rec.orderId));
        }
        orders_.emplace(rec.orderId, std::move(rec));
    }
}

void JsonDocumentStore::SaveAtomic() const {
    JsonValue::Array sampleArray;
    for (const auto& [id, rec] : samples_) sampleArray.push_back(EncodeSample(rec));
    JsonValue::Array orderArray;
    for (const auto& [id, rec] : orders_) orderArray.push_back(EncodeOrder(rec));

    JsonValue doc = JsonValue::MakeObject();
    doc.Set("schemaVersion", JsonValue::MakeNumber(kSchemaVersion));
    doc.Set("samples", JsonValue::MakeArray(std::move(sampleArray)));
    doc.Set("orders", JsonValue::MakeArray(std::move(orderArray)));

    const std::string tmpPath = filePath_ + ".tmp";
    const std::string backupPath = filePath_ + ".bak";

    WriteWholeFile(tmpPath, doc.Dump());

    std::error_code ec;
    if (fs::exists(filePath_, ec)) {
        fs::rename(filePath_, backupPath, ec);  // ADR-R8 — 교체 전 직전 정상본을 백업으로 보존
        if (ec) {
            throw domain::StorageUnavailable("cannot back up previous document: " + ec.message());
        }
    }
    fs::rename(tmpPath, filePath_, ec);  // 동일 볼륨 내 원자적 교체
    if (ec) {
        throw domain::StorageUnavailable("cannot atomically replace document: " + ec.message());
    }
}

std::optional<domain::SampleRecord> JsonDocumentStore::FindSampleById(const std::string& sampleId) const {
    EnsureLoaded();
    auto it = samples_.find(sampleId);
    if (it == samples_.end()) return std::nullopt;
    return it->second;
}

std::vector<domain::SampleRecord> JsonDocumentStore::FindAllSamples() const {
    EnsureLoaded();
    std::vector<domain::SampleRecord> result;
    result.reserve(samples_.size());
    for (const auto& [id, rec] : samples_) result.push_back(rec);
    return result;
}

domain::WriteOutcome JsonDocumentStore::AddSample(const domain::SampleRecord& sample) {
    EnsureLoaded();
    if (samples_.contains(sample.sampleId)) return domain::WriteOutcome::DuplicateKey;
    samples_.emplace(sample.sampleId, sample);
    SaveAtomic();
    return domain::WriteOutcome::Ok;
}

domain::WriteOutcome JsonDocumentStore::UpdateSample(const domain::SampleRecord& sample) {
    EnsureLoaded();
    auto it = samples_.find(sample.sampleId);
    if (it == samples_.end()) return domain::WriteOutcome::NotFound;
    it->second = sample;
    SaveAtomic();
    return domain::WriteOutcome::Ok;
}

domain::WriteOutcome JsonDocumentStore::DeleteSample(const std::string& sampleId) {
    EnsureLoaded();
    auto it = samples_.find(sampleId);
    if (it == samples_.end()) return domain::WriteOutcome::NotFound;
    samples_.erase(it);
    SaveAtomic();
    return domain::WriteOutcome::Ok;
}

std::optional<domain::OrderRecord> JsonDocumentStore::FindOrderById(int64_t orderId) const {
    EnsureLoaded();
    auto it = orders_.find(orderId);
    if (it == orders_.end()) return std::nullopt;
    return it->second;
}

std::vector<domain::OrderRecord> JsonDocumentStore::FindAllOrders() const {
    EnsureLoaded();
    std::vector<domain::OrderRecord> result;
    result.reserve(orders_.size());
    for (const auto& [id, rec] : orders_) result.push_back(rec);
    return result;
}

std::vector<domain::OrderRecord> JsonDocumentStore::FindOrdersByStatus(domain::OrderStatus status) const {
    EnsureLoaded();
    std::vector<domain::OrderRecord> result;
    for (const auto& [id, rec] : orders_) {
        if (rec.status == status) result.push_back(rec);
    }
    return result;
}

std::vector<domain::OrderRecord> JsonDocumentStore::FindOrdersBySampleId(const std::string& sampleId) const {
    EnsureLoaded();
    std::vector<domain::OrderRecord> result;
    for (const auto& [id, rec] : orders_) {
        if (rec.sampleId == sampleId) result.push_back(rec);
    }
    return result;
}

domain::WriteOutcome JsonDocumentStore::AddOrder(const domain::OrderRecord& order) {
    EnsureLoaded();
    if (orders_.contains(order.orderId)) return domain::WriteOutcome::DuplicateKey;
    orders_.emplace(order.orderId, order);
    SaveAtomic();
    return domain::WriteOutcome::Ok;
}

domain::WriteOutcome JsonDocumentStore::UpdateOrder(const domain::OrderRecord& order) {
    EnsureLoaded();
    auto it = orders_.find(order.orderId);
    if (it == orders_.end()) return domain::WriteOutcome::NotFound;
    it->second = order;
    SaveAtomic();
    return domain::WriteOutcome::Ok;
}

}  // namespace infra::persistence
