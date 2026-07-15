#include "MiniJson.h"

#include <cctype>
#include <cmath>
#include <cstdio>

namespace infra::json {

JsonValue JsonValue::MakeBool(bool value) {
    JsonValue v;
    v.type_ = Type::Boolean;
    v.boolValue_ = value;
    return v;
}

JsonValue JsonValue::MakeNumber(double value) {
    JsonValue v;
    v.type_ = Type::Number;
    v.numberValue_ = value;
    return v;
}

JsonValue JsonValue::MakeString(std::string value) {
    JsonValue v;
    v.type_ = Type::String;
    v.stringValue_ = std::move(value);
    return v;
}

JsonValue JsonValue::MakeArray(Array value) {
    JsonValue v;
    v.type_ = Type::Array;
    v.arrayValue_ = std::move(value);
    return v;
}

JsonValue JsonValue::MakeObject(Object value) {
    JsonValue v;
    v.type_ = Type::Object;
    v.objectValue_ = std::move(value);
    return v;
}

bool JsonValue::AsBool() const {
    if (type_ != Type::Boolean) throw std::runtime_error("JsonValue: not a boolean");
    return boolValue_;
}

double JsonValue::AsNumber() const {
    if (type_ != Type::Number) throw std::runtime_error("JsonValue: not a number");
    return numberValue_;
}

const std::string& JsonValue::AsString() const {
    if (type_ != Type::String) throw std::runtime_error("JsonValue: not a string");
    return stringValue_;
}

const JsonValue::Array& JsonValue::AsArray() const {
    if (type_ != Type::Array) throw std::runtime_error("JsonValue: not an array");
    return arrayValue_;
}

JsonValue::Array& JsonValue::AsArray() {
    if (type_ != Type::Array) throw std::runtime_error("JsonValue: not an array");
    return arrayValue_;
}

const JsonValue::Object& JsonValue::AsObject() const {
    if (type_ != Type::Object) throw std::runtime_error("JsonValue: not an object");
    return objectValue_;
}

JsonValue::Object& JsonValue::AsObject() {
    if (type_ != Type::Object) throw std::runtime_error("JsonValue: not an object");
    return objectValue_;
}

const JsonValue* JsonValue::Find(const std::string& key) const {
    if (type_ != Type::Object) throw std::runtime_error("JsonValue: not an object");
    for (const auto& [k, v] : objectValue_) {
        if (k == key) return &v;
    }
    return nullptr;
}

void JsonValue::Set(const std::string& key, JsonValue value) {
    if (type_ != Type::Object) throw std::runtime_error("JsonValue: not an object");
    for (auto& [k, v] : objectValue_) {
        if (k == key) {
            v = std::move(value);
            return;
        }
    }
    objectValue_.emplace_back(key, std::move(value));
}

namespace {

void DumpString(const std::string& s, std::string& out) {
    out += '"';
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    out += '"';
}

void DumpNumber(double value, std::string& out) {
    if (value == static_cast<long long>(value)) {
        out += std::to_string(static_cast<long long>(value));
    } else {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%g", value);
        out += buf;
    }
}

class Parser {
public:
    explicit Parser(const std::string& text) : text_(text), pos_(0) {}

    JsonValue ParseDocument() {
        SkipWhitespace();
        JsonValue v = ParseValue();
        SkipWhitespace();
        if (pos_ != text_.size()) {
            throw std::runtime_error("JSON: trailing data after document");
        }
        return v;
    }

private:
    const std::string& text_;
    size_t pos_;

    char Peek() {
        if (pos_ >= text_.size()) throw std::runtime_error("JSON: unexpected end of input");
        return text_[pos_];
    }

    char Next() {
        char c = Peek();
        ++pos_;
        return c;
    }

    void Expect(char expected) {
        if (Next() != expected) throw std::runtime_error(std::string("JSON: expected '") + expected + "'");
    }

    void SkipWhitespace() {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) ++pos_;
    }

    JsonValue ParseValue() {
        SkipWhitespace();
        char c = Peek();
        switch (c) {
            case '{': return ParseObject();
            case '[': return ParseArray();
            case '"': return JsonValue::MakeString(ParseString());
            case 't':
            case 'f': return ParseBool();
            case 'n': return ParseNull();
            default: return ParseNumber();
        }
    }

    JsonValue ParseObject() {
        Expect('{');
        JsonValue::Object obj;
        SkipWhitespace();
        if (Peek() == '}') {
            Next();
            return JsonValue::MakeObject(std::move(obj));
        }
        while (true) {
            SkipWhitespace();
            std::string key = ParseString();
            SkipWhitespace();
            Expect(':');
            JsonValue value = ParseValue();
            obj.emplace_back(std::move(key), std::move(value));
            SkipWhitespace();
            char c = Next();
            if (c == ',') continue;
            if (c == '}') break;
            throw std::runtime_error("JSON: expected ',' or '}' in object");
        }
        return JsonValue::MakeObject(std::move(obj));
    }

    JsonValue ParseArray() {
        Expect('[');
        JsonValue::Array arr;
        SkipWhitespace();
        if (Peek() == ']') {
            Next();
            return JsonValue::MakeArray(std::move(arr));
        }
        while (true) {
            arr.push_back(ParseValue());
            SkipWhitespace();
            char c = Next();
            if (c == ',') continue;
            if (c == ']') break;
            throw std::runtime_error("JSON: expected ',' or ']' in array");
        }
        return JsonValue::MakeArray(std::move(arr));
    }

    std::string ParseString() {
        Expect('"');
        std::string out;
        while (true) {
            char c = Next();
            if (c == '"') break;
            if (c == '\\') {
                char esc = Next();
                switch (esc) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    case 'u': {
                        if (pos_ + 4 > text_.size()) throw std::runtime_error("JSON: bad \\u escape");
                        unsigned code = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = Next();
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= (h - '0');
                            else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                            else throw std::runtime_error("JSON: bad \\u escape digit");
                        }
                        // ASCII 범위(스키마 필드가 요구하는 값)만 지원 — 그 외는 손실 없이 UTF-8 바이트 그대로 보존하지 않음.
                        if (code < 0x80) out += static_cast<char>(code);
                        break;
                    }
                    default: throw std::runtime_error("JSON: bad escape");
                }
            } else {
                out += c;
            }
        }
        return out;
    }

    JsonValue ParseBool() {
        if (text_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            return JsonValue::MakeBool(true);
        }
        if (text_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            return JsonValue::MakeBool(false);
        }
        throw std::runtime_error("JSON: bad literal");
    }

    JsonValue ParseNull() {
        if (text_.compare(pos_, 4, "null") == 0) {
            pos_ += 4;
            return JsonValue::MakeNull();
        }
        throw std::runtime_error("JSON: bad literal");
    }

    JsonValue ParseNumber() {
        size_t start = pos_;
        if (Peek() == '-') Next();
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        if (pos_ < text_.size() && text_[pos_] == '.') {
            ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) ++pos_;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) ++pos_;
        }
        if (pos_ == start) throw std::runtime_error("JSON: expected value");
        return JsonValue::MakeNumber(std::stod(text_.substr(start, pos_ - start)));
    }
};

}  // namespace

void JsonValue::DumpTo(std::string& out) const {
    switch (type_) {
        case Type::Null:
            out += "null";
            break;
        case Type::Boolean:
            out += boolValue_ ? "true" : "false";
            break;
        case Type::Number:
            DumpNumber(numberValue_, out);
            break;
        case Type::String:
            DumpString(stringValue_, out);
            break;
        case Type::Array: {
            out += '[';
            bool first = true;
            for (const auto& v : arrayValue_) {
                if (!first) out += ',';
                first = false;
                v.DumpTo(out);
            }
            out += ']';
            break;
        }
        case Type::Object: {
            out += '{';
            bool first = true;
            for (const auto& [k, v] : objectValue_) {
                if (!first) out += ',';
                first = false;
                DumpString(k, out);
                out += ':';
                v.DumpTo(out);
            }
            out += '}';
            break;
        }
    }
}

std::string JsonValue::Dump() const {
    std::string out;
    DumpTo(out);
    return out;
}

JsonValue JsonValue::Parse(const std::string& text) {
    Parser parser(text);
    return parser.ParseDocument();
}

}  // namespace infra::json
