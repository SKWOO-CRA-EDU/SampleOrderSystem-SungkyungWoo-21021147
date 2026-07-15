# AUDIT — CONTRACT v2 오류 표현력 감사 (B-02 재보고)

> **감사일**: 2026-07-15
> **대상**: `docs/CONTRACT.md` §5 저장소(Repository) 인터페이스
> **감사 축**: "SPEC 문장이 계약에 반영되었는가"가 아니라 **"설계상 필연적으로 발생하는 상황을 계약이 표현할 수 있는가"**.
> **범위 제한**: 이 문서는 감사 결과만 기록한다. CONTRACT.md는 이 문서로 인해 수정되지 않았으며, 해법도 제안하지 않는다.

---

## 1. CONTRACT.md §5 인터페이스 메서드 전체 목록 (12개)

### ISampleRepository (6개)
1. `FindById(sampleId) -> optional<SampleRecord>`
2. `FindAll() -> vector<SampleRecord>`
3. `Exists(sampleId) -> bool`
4. `Add(sample) -> void`
5. `Update(sample) -> void`
6. `Delete(sampleId) -> bool`

### IOrderRepository (6개)
7. `FindById(orderId) -> optional<OrderRecord>`
8. `FindAll() -> vector<OrderRecord>`
9. `FindByStatus(status) -> vector<OrderRecord>`
10. `NextOrderId() -> int64_t`
11. `Add(order) -> void`
12. `Update(order) -> void`

---

## 2~3. 메서드별 실패 상황 및 전달 가능 여부

감사 축: 대상의 존재/부재 · 키의 중복 · 저장된 데이터를 신뢰할 수 없는 경우 · 스키마 버전이 다른 경우 · 저장 매체에 접근할 수 없는 경우 · 기타 발견사항.

| # | 메서드 | 실패 상황 (축) |
|---|---|---|
|1| Sample.FindById | 대상 부재 |
|2| Sample.FindById | 저장소에 sampleId 중복 레코드 존재 (키 중복) |
|3| Sample.FindById | 해당 레코드 필드 파싱 실패/타입 불일치 (신뢰 불가 데이터) |
|4| Sample.FindById | 파일의 schemaVersion이 현재와 다름 |
|5| Sample.FindById | 저장 매체 접근 불가 (파일 잠김/권한 없음/디스크 오류) |
|6| Sample.FindAll | 대상 부재 (시료 0건) |
|7| Sample.FindAll | 키 중복 |
|8| Sample.FindAll | 신뢰 불가 데이터 (일부 레코드 손상) |
|9| Sample.FindAll | 스키마 버전 다름 |
|10| Sample.FindAll | 저장 매체 접근 불가 |
|11| Sample.Exists | 대상 부재 |
|12| Sample.Exists | 신뢰 불가 데이터 (해당 레코드 파싱 실패) |
|13| Sample.Exists | 스키마 버전 다름 (구버전 포맷이라 매칭 실패) |
|14| Sample.Exists | 저장 매체 접근 불가 |
|15| Sample.Add | 키 중복 (이미 존재하는 sampleId로 Add) |
|16| Sample.Add | 저장 매체 접근 불가 (쓰기 실패) |
|17| Sample.Add | 신뢰 불가 데이터 (직렬화 실패) |
|18| Sample.Add | 스키마 버전 다름 (기존 파일과 새 레코드 포맷 불일치) |
|19| Sample.Update | 대상 부재 (이미 삭제된 sampleId Update) |
|20| Sample.Update | 저장 매체 접근 불가 / 신뢰 불가 데이터 / 스키마 버전 |
|21| Sample.Delete | 대상 부재 |
|22| Sample.Delete | 참조무결성 위반 (Order가 참조 중) |
|23| Sample.Delete | 키 중복 (동일 sampleId 레코드 2개 이상) |
|24| Sample.Delete | 저장 매체 접근 불가 |
|25| Sample.Delete | 신뢰 불가 데이터 / 스키마 버전 |
|26| Order.FindById | 대상 부재 |
|27| Order.FindById | 키 중복 (orderId 충돌 — 이론상 자동증가라 불가능해야 하나 NextOrderId 오류·복구 시 발생 가능) |
|28| Order.FindById | 신뢰 불가 데이터 / 스키마 버전 / 저장 매체 접근 불가 |
|29| Order.FindAll | 대상 부재 (0건) / 키 중복 / 신뢰 불가 데이터 / 스키마 버전 / 저장 매체 접근 불가 |
|30| Order.FindByStatus | 대상 부재 (해당 상태 주문 0건) |
|31| Order.FindByStatus | §2.1 열거값 외 문자열 status 입력 (오탈자 등) |
|32| Order.FindByStatus | 저장된 레코드의 status 필드가 열거값 외 문자열 (손상/구버전) |
|33| Order.FindByStatus | 스키마 버전 다름 / 저장 매체 접근 불가 |
|34| Order.NextOrderId | 저장소 비어있을 때 초깃값 |
|35| Order.NextOrderId | 키 중복 위험 (백업 복원 등으로 카운터와 실제 최대 orderId 불일치) |
|36| Order.NextOrderId | 신뢰 불가 데이터 (저장된 orderId들이 비순차/음수 — 손상) |
|37| Order.NextOrderId | 저장 매체 접근 불가 |
|38| Order.Add | 키 중복 / 저장 매체 접근 불가 / 신뢰 불가 데이터 / 스키마 버전 |
|39| Order.Add | 존재하지 않는 sampleId를 참조하는 Order 생성 (§1.3 "등록 Sample 참조(존재검증)"의 검증 주체 불명) |
|40| Order.Update | 대상 부재 (이미 없는 orderId) |
|41| Order.Update | §4에 없는 불법 상태 전이 (예: RELEASE→RESERVED)로 Update 호출 |
|42| Order.Update | 신뢰 불가 데이터 / 스키마 버전 / 저장 매체 접근 불가 |

각 실패 상황의 전달 가능 여부와 호출자가 실제로 보게 되는 것은 아래 표로 이어서 기록한다.

| # | 전달가능여부 |
|---|---|
|1| 가능 |
|2| 불가능 |
|3| 불가능 |
|4| 불가능 |
|5| 불가능 |
|6| 가능 |
|7| 불가능 |
|8| 불가능 |
|9| 불가능 |
|10| 불가능 |
|11| 가능 |
|12| 불가능 |
|13| 불가능 |
|14| 불가능 |
|15| 불가능 |
|16| 불가능 |
|17| 불가능 |
|18| 불가능 |
|19| 불가능 |
|20| 불가능 |
|21| 가능 (계약 주석에 명시) |
|22| **불가능** |
|23| 불가능 |
|24| 불가능 |
|25| 불가능 |
|26| 가능 |
|27| 불가능 |
|28| 불가능 |
|29| 부재만 가능, 나머지 불가능 |
|30| 가능 |
|31| 불가능 |
|32| 불가능 |
|33| 불가능 |
|34| 계약에 값 자체는 암시(1)되나 오류 채널 자체가 없음 |
|35| 불가능 |
|36| 불가능 |
|37| 불가능 |
|38| 불가능 |
|39| 불가능 |
|40| 불가능 |
|41| 불가능 |
|42| 불가능 |

| # | 호출자가 실제로 보게 되는 것 |
|---|---|
|1| `optional`이 empty — 설계된 정상 경로 |
|2| 어느 레코드가 반환됐는지, 중복이 있었는지 알 방법 없음 — `Exists`/`FindAll` 결과와 모순돼도 감지 불가 |
|3| `optional<SampleRecord>`엔 "찾았는데 손상됨" 상태가 없음 → 구현별로 조용히 기본값 채움 / 예외 / `nullopt`(=없음과 혼동) 중 무엇이든 가능, 계약은 규정 안 함 |
|4| 마이그레이션 여부·실패를 알 길 없음. 구버전 필드 누락 시 3번과 동일하게 조용히 처리되거나 크래시 |
|5| `nullopt` 반환 시 "존재하지 않음"과 완전히 동일하게 보임 — IO 오류가 "no such sample"로 둔갑 |
|6| 빈 vector — 정상 경로 |
|7| 중복 레코드가 그대로 두 개 들어있는 vector 반환 — 호출자는 이것이 손상인지 정상인지 판단 근거 없음 |
|8| 전체 실패(예외)로 갈지 손상 레코드만 스킵할지 정의 없음 — 스킵되면 "시료가 원래 적었나보다"로 오인 |
|9| 8번과 동일 |
|10| 빈 vector 반환 시 6번(정상적으로 0건)과 구분 불가능 |
|11| `false` — 설계된 정상 경로 |
|12| "존재 여부 판단 불가"를 표현할 방법이 `bool`엔 없음 — false로 뭉개짐 |
|13| false로 뭉개짐 — FR-33 삭제 가드가 "참조 있음"을 "참조 없음"으로 오판할 위험 |
|14| false로 뭉개짐 — 위와 동일한 위험 경로 |
|15| `void` — 성공했는지, 덮어썼는지, 무시됐는지, 예외가 났는지 호출자는 전혀 알 수 없음 |
|16| `void` 반환 후 정상 종료로 보임 — 등록됐다고 믿지만 실제로 저장 안 된 silent data loss |
|17| 15/16과 동일 |
|18| 규정 없음 |
|19| `void` — upsert인지 no-op인지 실패인지 알 수 없음 |
|20| 15~18과 동일 |
|21| `false` |
|22| `false` — 21번(대상 없음)과 동일한 값. FR-33이 요구하는 "명확한 거부 메시지"를 만들 정보가 `bool` 안에 없음. Model이 별도로 사전 체크해야 하는데 그 체크에 쓸 `IOrderRepository::FindBySampleId` 류 메서드 자체가 §5에 없음 |
|23| 몇 건이 삭제됐는지 `bool`로는 표현 불가 — 하나만 지워지고 "성공"으로 보일 수 있음 |
|24| `false` 반환 시 21번(대상 없음)과 구분 불가 — 삭제 실패가 "애초에 없었다"로 오인되어 재시도 로직이 잘못된 판단을 할 수 있음 |
|25| 동일 패턴 |
|26| `nullopt` |
|27| 어느 레코드가 반환되는지 불명 |
|28| Sample.FindById와 동일 패턴, `nullopt`이 "없음"과 혼동 |
|29| Sample.FindAll과 동일 |
|30| 빈 vector |
|31| 빈 vector — "그 상태 주문 없음"과 "잘못된 상태 문자열 조회"가 구분 불가 |
|32| 해당 레코드는 어떤 `FindByStatus` 호출로도 찾을 수 없는 "유령 주문"이 됨. `FindAll`에는 나타나지만 상태별 조회 어디에도 안 걸림 |
|33| 빈 vector로 뭉개짐 — 30번과 구분 불가 |
|34| 정상 케이스 자체는 동작 가능 |
|35| `int64_t` 하나만 반환 — "이 값이 이미 사용 중일 수 있다"는 경고를 표현할 자리가 없음. Add 시 그대로 중복 orderId 생성 위험 |
|36| max+1 로직이 무엇을 반환할지 계약 미정, 오류 신호 없음 |
|37| 임의값(0 또는 1) 반환 시 정상값으로 오인 — 이후 Add에서 orderId 충돌 유발 가능 |
|38| Sample.Add와 동일 패턴 (void라 전부 불가) |
|39| `void` — 이 검증이 Repository 책임인지 Model 책임인지, 위반 시 무엇이 반환되는지 계약에 없음 |
|40| `void` — no-op인지 실패인지 구분 불가 |
|41| Repository는 순수 저장 계층으로 추정되나 계약에 "검증 안 함"이 명문화되어 있지 않음 — Update가 그대로 받아들이는지 불명 |
|42| Add와 동일 패턴 |

---

## 4. 공존하는 오류 관용구

| 관용구 | 사용처 | 오류 표현력 |
|---|---|---|
| `void` | Sample.Add/Update, Order.Add/Update (4개) | 없음 — 성공/실패 신호 자체가 없음 |
| `bool` | Sample.Exists, Sample.Delete (2개) | "예/아니오"만 있고, `Delete`의 경우 "대상 없음"과 "정책적 거부"가 동일 값으로 겹침 |
| `optional<T>` | Sample/Order.FindById (2개) | "찾음/못 찾음"만 있고, "찾았지만 손상됨"·"IO 오류로 못 찾은 척"이 "없음"과 겹침 |
| `vector<T>` (암묵적) | Sample/Order.FindAll, Order.FindByStatus (3개) | "결과 있음/0건"만 있고, "0건"과 "오류로 못 읽음"이 겹침 |

관용구 4종이 공존하지만, 넷 중 어느 것도 오류 전용 채널로 설계되지 않았다. 예외(exception)는 시그니처 어디에도 선언되어 있지 않다 — `noexcept` 표기도 없고 예외 사양도 없다. 즉 실제로는 "정의된 관용구 4개 + 정의되지 않은 5번째 채널(구현마다 다를 예외/로그/무시)"이 공존하며, 이 5번째가 4개 PoC 저장소마다 다르게 구현될 경우 동일 계약을 배포했음에도 오류 시 동작이 저장소마다 달라질 것이다.

---

## 5. ADR-C2 "호출자" 명시 여부

- `docs/PRD.md` FR-33: "호출자"라는 단어 자체가 없다. Given/When/Then에도 어느 컴포넌트가 참조무결성을 확인하는지 언급 없음.
- `docs/CONTRACT.md` §5, `Delete` 선언부 주석: "참조무결성 검증(ADR-C2)은 호출자(Model/Controller) 책임" — 여기서만 유일하게 언급되지만, "Model/Controller"로 두 컴포넌트를 병기하고 있어 어느 쪽이 실제 책임자인지 특정하지 않는다. CLAUDE.md의 MVC 규칙("Controller는 조율만, 도메인 로직은 Model")에 따르면 참조무결성 검증은 도메인 로직이므로 Model이어야 할 것으로 추정되지만, 이는 CLAUDE.md 규칙으로부터의 추론이지 CONTRACT.md/PRD.md 자체의 명시가 아니다.
- 또한 이 "호출자"가 참조무결성을 확인할 때 사용할 메서드 자체가 §5에 없다 — `IOrderRepository`에는 `FindBySampleId` 류가 없고, `FindByStatus`/`FindById`/`FindAll`뿐이다. 즉 호출자는 명시되지 않았을 뿐 아니라, 명시되었다 하더라도 그 호출자가 무엇을 호출해서 참조무결성을 확인해야 하는지도 계약에 없다 (아마 `FindAll()` 후 애플리케이션 레벨 필터링을 암묵적으로 가정).

**결론: 명시되어 있지 않다.** CONTRACT.md의 주석 한 줄이 "Model/Controller"라고 모호하게 언급할 뿐, 단일 책임 주체도 검증에 쓸 메서드도 특정되어 있지 않다.

---

## 6. 닫힘 방식 (CONTRACT v3 대조, 전체 42행)

> 감사 기준 개정: "모든 실패 상황이 시그니처로 전달 가능해야 한다"는 기준을 폐기하고, 닫힘 방식을
> **(a) 표현한다**(오류 어휘로 호출자에게 전달) / **(b) 불가능하게 만든다**(타입·메서드 제거로 상태 자체를 표현 불가능하게) /
> **(c) 범위 밖으로 선언한다**(계약에 명문화 + 근거 기록) 셋 중 하나로 재정의한다. (c)도 닫힘이다.
> 아래 표는 `docs/DECISIONS.md`의 ADR-E1~E11 및 `docs/CONTRACT.md` v3을 근거로 42행 전체를 닫는다.

| # | 메서드 / 실패 상황 | 닫힘 방식 | 근거 ADR-ID | 설명 |
|---|---|---|---|---|
| 1 | Sample.FindById / 대상 부재 | (a) 표현 | ADR-E1 | `optional` empty가 [R] 범주의 설계된 정상 경로. |
| 2 | Sample.FindById / 키 중복(저장소에 중복 레코드) | (c) 범위 밖 선언 | ADR-E11, ADR-E9 | 정상 경로는 `Add`의 `DuplicateKey`로 이미 불가능(보조: (b)); 외부 파일 조작으로 생긴 중복의 완전 탐지 보장은 비범위로 명문화, 탐지 시 `StorageCorrupted`로 분류. |
| 3 | Sample.FindById / 데이터 신뢰 불가(파싱 실패) | (a) 표현 | ADR-E2 | `StorageCorrupted` 예외로 표현. |
| 4 | Sample.FindById / 스키마 버전 다름 | (a) 표현 | ADR-E2 | `SchemaVersionMismatch` 예외로 표현. |
| 5 | Sample.FindById / 저장 매체 접근 불가 | (a) 표현 | ADR-E2 | `StorageUnavailable` 예외로 표현. |
| 6 | Sample.FindAll / 대상 부재(0건) | (a) 표현 | ADR-E1 | 빈 `vector`가 [R] 범주의 설계된 정상 경로. |
| 7 | Sample.FindAll / 키 중복 | (c) 범위 밖 선언 | ADR-E11, ADR-E9 | #2와 동일 패턴. |
| 8 | Sample.FindAll / 신뢰 불가 데이터(일부 손상) | (a) 표현 | ADR-E2, ADR-E11 | `StorageCorrupted` 예외 + "부분 손상 시 전체 손상 간주"(비범위 §9)로 조용한 스킵 자체를 금지. |
| 9 | Sample.FindAll / 스키마 버전 다름 | (a) 표현 | ADR-E2 | `SchemaVersionMismatch` 예외로 표현. |
| 10 | Sample.FindAll / 저장 매체 접근 불가 | (a) 표현 | ADR-E2 | `StorageUnavailable` 예외로 표현. |
| 11 | Sample.Exists / 대상 부재 | (b) 불가능화 | ADR-E3 | `Exists` 메서드 자체를 계약에서 제거. `FindById().has_value()`로 대체. |
| 12 | Sample.Exists / 신뢰 불가 데이터 | (b) 불가능화 | ADR-E3 | 메서드 제거로 `bool` 표현력 문제 자체가 소멸. |
| 13 | Sample.Exists / 스키마 버전 다름 | (b) 불가능화 | ADR-E3 | 동일. |
| 14 | Sample.Exists / 저장 매체 접근 불가 | (b) 불가능화 | ADR-E3 | 동일. |
| 15 | Sample.Add / 키 중복 | (a) 표현 | ADR-E1, ADR-E2 | `WriteOutcome::DuplicateKey`로 표현. |
| 16 | Sample.Add / 저장 매체 접근 불가(쓰기 실패) | (a) 표현 | ADR-E2 | `StorageUnavailable` 예외로 표현. |
| 17 | Sample.Add / 신뢰 불가 데이터(직렬화 실패) | (a) 표현 | ADR-E2 | `StorageCorrupted` 예외로 표현. |
| 18 | Sample.Add / 스키마 버전 다름 | (a) 표현 | ADR-E2 | `SchemaVersionMismatch` 예외로 표현. |
| 19 | Sample.Update / 대상 부재 | (a) 표현 | ADR-E1, ADR-E2 | `WriteOutcome::NotFound`로 표현. |
| 20 | Sample.Update / 저장매체·신뢰불가·스키마버전 | (a) 표현 | ADR-E2 | 3개 예외로 표현. |
| 21 | Sample.Delete / 대상 부재 | (a) 표현 | ADR-E1, ADR-E2 | `bool`을 폐기하고 `WriteOutcome::NotFound`로 표현. |
| 22 | Sample.Delete / 참조무결성 위반 | (c) 범위 밖 선언 | ADR-E8, ADR-E9 | Repository 계약 관점에서는 참조무결성을 검증하지 않음을 명문화(비범위). 실제 거부는 Model이 `Delete` 호출 전 `FindBySampleId`로 확인 후 View를 통해 표현(수임자+메서드 §5.3에 명시). |
| 23 | Sample.Delete / 키 중복(동일 sampleId 2개 이상) | (c) 범위 밖 선언 | ADR-E11, ADR-E9 | #2와 동일 패턴. |
| 24 | Sample.Delete / 저장 매체 접근 불가 | (a) 표현 | ADR-E2 | `StorageUnavailable` 예외로 표현. |
| 25 | Sample.Delete / 신뢰불가·스키마버전 | (a) 표현 | ADR-E2 | 예외로 표현. |
| 26 | Order.FindById / 대상 부재 | (a) 표현 | ADR-E1 | `optional` empty. |
| 27 | Order.FindById / 키 중복(orderId 충돌) | (b) 불가능화 | ADR-E4 | `NextOrderId` 제거, 채번은 Model이 `FindAll()` 기반 max+1 수행, `Add`의 `DuplicateKey`가 최종 방어선 — 정상 경로에서 구조적으로 불가능. |
| 28 | Order.FindById / 신뢰불가·스키마버전·저장매체 | (a) 표현 | ADR-E2 | 3개 예외로 표현. |
| 29 | Order.FindAll / 대상부재(0건)·키중복·신뢰불가·스키마버전·저장매체 | (a) 표현 | ADR-E1, ADR-E2 | 0건은 빈 `vector`(a), 신뢰불가/스키마버전/저장매체는 예외(a)로 표현. 키중복 하위 사례는 #2/#7과 동일하게 (c)로 보조 처리(ADR-E11). |
| 30 | Order.FindByStatus / 대상부재(0건) | (a) 표현 | ADR-E1 | 빈 `vector`. |
| 31 | Order.FindByStatus / 열거값 외 문자열 입력(오탈자) | (b) 불가능화 | ADR-E6 | 인자를 `std::string`→`OrderStatus` 열거형으로 교체 — 오탈자 자체가 컴파일 타임에 불가능. |
| 32 | Order.FindByStatus / 저장된 status가 열거값 외 문자열(손상/구버전) | (a) 표현 | ADR-E10, ADR-E2 | 불변식("저장된 status는 §2 열거값 중 하나") 위반 시 `StorageCorrupted`로 표현 — "유령 주문" 상태가 감지 가능한 손상으로 격상. |
| 33 | Order.FindByStatus / 스키마버전·저장매체 | (a) 표현 | ADR-E2 | 예외로 표현. |
| 34 | Order.NextOrderId / 저장소 비어있을 때 초깃값 | (b) 불가능화 | ADR-E4 | `NextOrderId` 메서드 자체 제거 — 초깃값 문제가 Repository 계약에서 소멸(Model이 빈 경우 1로 시작). |
| 35 | Order.NextOrderId / 키중복 위험(카운터·실제 불일치) | (b) 불가능화 | ADR-E4 | 메서드 제거로 "카운터"라는 개념 자체가 Repository에서 소멸. `Add`의 `DuplicateKey`가 최종 방어선. |
| 36 | Order.NextOrderId / 신뢰불가 데이터(비순차/음수) | (b) 불가능화 | ADR-E4 | 동일. |
| 37 | Order.NextOrderId / 저장 매체 접근 불가 | (b) 불가능화 | ADR-E4 | 메서드 제거. 채번에 쓰이는 `FindAll()`이 `StorageUnavailable`을 던져 표면화(임의값 반환 경로 소멸). |
| 38 | Order.Add / 키중복·저장매체·신뢰불가·스키마버전 | (a) 표현 | ADR-E1, ADR-E2 | `WriteOutcome::DuplicateKey` + 3개 예외로 표현. |
| 39 | Order.Add / 존재하지 않는 sampleId 참조(검증주체 불명) | (c) 범위 밖 선언 | ADR-E9 | Repository는 참조무결성을 검증하지 않음을 명문화(§5.4). 검증 주체는 Model(PRD FR-29)로 계약에 명시. |
| 40 | Order.Update / 대상 부재 | (a) 표현 | ADR-E1, ADR-E2 | `WriteOutcome::NotFound`로 표현. |
| 41 | Order.Update / §4에 없는 불법 상태 전이 시도 | (c) 범위 밖 선언 | ADR-E9 | Repository는 상태 전이 적법성을 검증하지 않음을 명문화(§5.4/§4). 검증 주체는 Model(PRD FR-27). |
| 42 | Order.Update / 신뢰불가·스키마버전·저장매체 | (a) 표현 | ADR-E2 | 3개 예외로 표현. |

### 자체 검증

- (a) 표현으로 닫힌 행: 1,3,4,5,6,8,9,10,15,16,17,18,19,20,21,24,25,26,28,29,30,32,33,38,40,42 = **26행**
- (b) 불가능화로 닫힌 행: 11,12,13,14,27,31,34,35,36,37 = **10행**
- (c) 범위 밖 선언으로 닫힌 행: 2,7,22,23,39,41 = **6행**
- **합계: 26 + 10 + 6 = 42행 — 42행 전체와 일치. 빈 칸 없음.**
