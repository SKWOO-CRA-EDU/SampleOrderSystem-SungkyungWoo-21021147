// 계약에 없는 내부 유틸리티(CONTRACT.md §3 "파일 형식은 저장소 자유"에 속한다).
// RootDocument(§3) 하나를 표현하기에 충분한 최소 JSON 값 모델 — 객체/배열/문자열/숫자/불/null만 지원한다.
// 파싱 실패는 std::runtime_error를 던진다. 호출자(JsonDocumentStore)가 이를 domain::StorageCorrupted로 재포장한다.
#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace infra::json {

class JsonValue {
public:
    enum class Type { Null, Boolean, Number, String, Array, Object };
    using Array = std::vector<JsonValue>;
    using Object = std::vector<std::pair<std::string, JsonValue>>;

    JsonValue() : type_(Type::Null) {}

    static JsonValue MakeNull() { return JsonValue(); }
    static JsonValue MakeBool(bool value);
    static JsonValue MakeNumber(double value);
    static JsonValue MakeString(std::string value);
    static JsonValue MakeArray(Array value = {});
    static JsonValue MakeObject(Object value = {});

    Type GetType() const { return type_; }

    bool AsBool() const;
    double AsNumber() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    Array& AsArray();
    const Object& AsObject() const;
    Object& AsObject();

    // 객체 전용 헬퍼. 키가 없으면 nullptr.
    const JsonValue* Find(const std::string& key) const;
    void Set(const std::string& key, JsonValue value);

    std::string Dump() const;

    // 파싱 실패 시 std::runtime_error.
    static JsonValue Parse(const std::string& text);

private:
    Type type_;
    bool boolValue_ = false;
    double numberValue_ = 0.0;
    std::string stringValue_;
    Array arrayValue_;
    Object objectValue_;

    void DumpTo(std::string& out) const;
};

}  // namespace infra::json
