// 계약에 없는 내부 유틸리티. CONTRACT.md v4 §2 매핑표를 그대로 코드로 옮긴 것뿐이며,
// wire format(§3, 문자열)과 in-memory 표현(domain::OrderStatus) 사이의 변환은 스키마 마이그레이션이 아니다.
#pragma once

#include <string>

#include "../../domain/Records.h"

namespace infra::persistence {

// CONTRACT §2 5개 값 중 하나가 아니면 std::runtime_error를 던진다.
// 호출자(JsonDocumentStore)가 domain::StorageCorrupted로 재포장한다(CONTRACT §6 불변식2/ADR-E10).
domain::OrderStatus DecodeOrderStatus(const std::string& stored);
std::string EncodeOrderStatus(domain::OrderStatus status);

}  // namespace infra::persistence
