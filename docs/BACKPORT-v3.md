# BACKPORT-v3 — 4개 PoC 계약 해석 drift 대조 원장 (조사 전용)

> 이 문서는 통합 계획이 아니다. 4개 PoC(ConsoleMVC, DataPersistence, DataMonitor,
> DummyDataGenerator)가 `docs/CONTRACT.md` v3(2026-07-15, 동결)를 각각 어떻게 읽었는지
> as-found 상태로 기록한 조사 원장이다. src diff 0 — 어떤 PoC 파일도 수정하지 않았고,
> 어떤 파일도 이 저장소로 복사하지 않았다. 본문의 "이식"은 ADR-I2(복사 후 수정 금지 —
> v3 기준 재작성)에 따른 재작성 대상 여부 판정이며, 파일 이동을 의미하지 않는다.
> 발번(FR/ADR 번호 신설)은 하지 않았다. PoC별로 절을 분리해, 각 절이 이후
> 해당 PoC의 POC-CLOSEOUT.md 원본이 될 수 있게 했다.

## 0. 전제 확인

- `docs/CONTRACT.md` 헤더: `버전: v3 | 최종 수정일: 2026-07-15`, 동결 선언 확인됨.
- ADR 실제 계열(DECISIONS.md 견출 기준): Q1~Q16 / R1~R10 / C1~C3 / E1~E11 / I1~I4 / H1~H4.
- 4개 PoC 경로 확인됨(DummyDataGenerator 경로 포함, 지시된 경로와 일치).
- ADR-I2: PoC의 v1 코드를 복사한 뒤 고치는 방식을 쓰지 않는다 — v3 기준 재작성.
- ADR-I3: DataMonitor의 "별도 프로세스가 동일 파일을 읽는" 프로세스 아키텍처만 통합 제외. 기능 자체는 제외 대상 아님.
- ADR-E4: IOrderRepository에 NextOrderId를 두지 않는다 — 채번은 Model 책임.

## 1. 종합 절 — 축1 최종 결론 (drift 확정 경위)

### 1.1 Records.h ↔ Contracts.h 의미 수렴 — 성과로 기록

ConsoleMVC(`src/domain/Records.h`)와 DataPersistence(`src/Contracts.h`)는 독립적으로
작성됐음에도 다음 전부에서 완전일치했다:
- `OrderStatus` 열거값 5개(Reserved/Rejected/Producing/Confirmed/Release)와 순서
- `WriteOutcome` 열거값 3개(Ok/NotFound/DuplicateKey)와 순서
- 예외 3종(StorageUnavailable/StorageCorrupted/SchemaVersionMismatch)의 이름, `std::runtime_error` 상속, 생성자 위임(`using std::runtime_error::runtime_error`) 방식
- 둘 다 네임스페이스 없이 전역에 선언

Contracts.h는 Records.h에 없는 `ISampleRepository`/`IOrderRepository` 선언까지 포함하는
**상위집합**이다 — "완전 중복이므로 버림"이라는 과거 세션 주장은 Records.h와 Contracts.h를
실제 diff한 근거 위에 있지 않았다(이번 조사에서 최초로 `diff -u --strip-trailing-cr -B`
실행 결과 확인).

### 1.2 갈라진 지점 — `[[nodiscard]]`, drift 확정

- `docs/CONTRACT.md:126`(§5.1 표): `[[nodiscard]] WriteOutcome`
- `docs/CONTRACT.md:132`(§5.2 코드 블록): `` [[nodiscard]] enum class WriteOutcome { Ok, NotFound, DuplicateKey }; ``
- DECISIONS.md ADR-E1(509~522행) 결정문: "[D] 도메인 거부 — 관용구: `[[nodiscard]] enum class WriteOutcome { Ok, NotFound, DuplicateKey }`"
- ConsoleMVC `Records.h:9`: `enum class [[nodiscard]] WriteOutcome { ... }` — 계약대로 부착.
- DataPersistence `Contracts.h:34`: `enum class WriteOutcome { ... }` — `[[nodiscard]]` 없음.

계약 본문 2곳(§5.1, §5.2)과 ADR-E1 결정문이 관용구 자체를 `[[nodiscard]] enum class
WriteOutcome`으로 명문화했으므로, DataPersistence/Contracts.h의 누락은 **계약 위반(drift)**이다.
표기 차이가 아니라 하중을 받는 부분이다: `[[nodiscard]]`가 없으면 `repo.Add(rec);`처럼
반환값을 버리는 호출이 조용히 컴파일되어 [D] 범주(도메인 거부)가 강제로 다뤄지지 않는다.
ADR-E1이 [D] 범주를 별도로 둔 이유 자체가 도메인 거부를 호출자가 반드시 처리하게 만드는
것이고, `[[nodiscard]]`가 그 강제 메커니즘이다 — 이것이 빠지면 범주의 보증이 사라진다.

독립 구현 둘은 의미론 전부(열거값 5+3개, 예외 3종의 이름·상속·생성자 위임)에서 수렴했다.
갈라진 것은 정확히 하나, 강제 장치(`[[nodiscard]]`)뿐이다. **계약은 의미를 완벽히
전달했고, 강제는 전달하지 못했다.**

### 1.3 감사 축의 한계

AUDIT-CONTRACT-v2.md는 인터페이스 12개 × 실패상황 42행을 빈 칸 0으로 닫았다. 그럼에도
이 drift는 샜다. 감사 축이 "표현할 수 있는가"였고 "표현이 강제되는가"가 아니었기 때문이다.
감사가 틀린 것이 아니다 — 계약의 감사였지 코드의 감사가 아니었다. 계약은 옳고 코드가 틀렸다.

### 1.4 DummyDataGenerator — drift 아닌 "미갱신(stale)"

`Contract.h`는 v1 형태(아래 §4 참조)다. 이는 같은 계약을 다르게 읽은 것(drift)이 아니라
계약을 읽지 않은 상태(미갱신)다. ADR-I2가 이미 "v3 기준 재작성"을 결정했으므로, v1
상태여도 재작성 대상이라는 사실 자체는 작업을 막지 않는다.

### 1.5 축1 재검토 중 발견한 추가 사실 — DataMonitor에도 미갱신 계약 파일 존재

최초 축1 조사(`*record*.h`/`*contract*.h`/`*contract*.md` 패턴 검색)에서 DataMonitor는
"계약 헤더 상당 파일 없음"으로 기록했으나, 축5 grep 과정에서 `DataMonitor/Domain.h`가
동일한 역할(SampleRecord/OrderRecord/ISampleRepository/IOrderRepository 선언)을 하고
있음을 확인했다. 파일명이 `Records`/`Contract` 계열이 아니어서(단순히 `Domain.h`) 최초
패턴 검색에 걸리지 않았을 뿐이다. `Domain.h` 헤더 주석은 "CONTRACT.md 5절을... 글자
그대로 옮긴 것"이라고 적혀 있으나, 실제 내용은 DummyDataGenerator/Contract.h와 같은 계열의
v1 형태(§4 표 참조)다. 즉 DataMonitor도 "미갱신" 상태다 — DataMonitor에 계약 헤더 파일이
아예 없다는 이전 기록은 정정한다. "계약 헤더 파일이 없다"가 아니라 "있지만 미갱신이다."

## 2. PoC별 절

### 2.1 ConsoleMVC

**계약 파일**: `src/domain/Records.h` — v3 완전 정합(§1.1, §1.2 참조).

**구조**: `src/app`(Controller), `src/infra/console`(View: ConsoleOutputAdapter/PlainConsoleOutputAdapter 2종), `src/infra/repository`(InMemorySampleRepository/InMemoryOrderRepository), `src/ports`(IInputPort/IOutputPort/ISampleRepository/IOrderRepository), `tests`(ControllerScenarioTests, fakes).

**오류 어휘 사용 시점**: `WriteOutcome`만 사용(Add/Update가 반환, AppController.cpp:65에서 분기). 예외 3종은 InMemory 저장소이므로 코드베이스 전체에서 **한 번도 throw되지 않음**(인메모리는 StorageUnavailable/StorageCorrupted/SchemaVersionMismatch가 발생할 실제 저장 매체가 없음).

**IOutputPort 경계**: `src/ports/IOutputPort.h` 주석이 "화면 문자열이 아니라 무슨 일이
일어났는지를 의미론적으로 전달한다"고 명시하고, "Print(std::string) 같은 범용 텍스트
메서드를 추가하지 말 것"이라고 경고함 — RecordingOutputPort(§3-(a))가 이 경계를 지키는지의 근거.

### 2.2 DataPersistence

**계약 파일**: `src/Contracts.h` — `[[nodiscard]]` 누락 1건(drift, §1.2).

**구조**: `src/FileRepositories.{h,cpp}`(디스크 구현), `src/InMemoryRepositories.{h,cpp}`,
`src/JsonDocumentStore.{h,cpp}`(RootDocument load/save 단일 소유, 예외 재포장 경계),
`src/MiniJson.{h,cpp}`(계약에 없는 내부 유틸), `src/OrderStatusCodec.{h,cpp}`(§3-(c) 참조).

**오류 어휘 사용 시점**: 6개 전부 사용.
- `WriteOutcome::Ok/NotFound/DuplicateKey` — `JsonDocumentStore.cpp`, `InMemoryRepositories.cpp` 양쪽에서 Add/Update/Delete 반환값으로 사용.
- `StorageUnavailable` — `JsonDocumentStore.cpp:44,51,162,165,170,174` (파일 열기 실패, atomic replace 실패 등).
- `StorageCorrupted` — `JsonDocumentStore.cpp:58,68,97,104,111,118` (파싱 실패, status 값 이상, 중복 레코드 등).
- `SchemaVersionMismatch` — `JsonDocumentStore.cpp:71` (지원하지 않는 schemaVersion).

### 2.3 DataMonitor

**계약 파일**: `Domain.h` — 미갱신(v1 형태, §1.5/§4 참조). ADR-I3: 프로세스 아키텍처만 통합 제외, 기능 자체는 제외 대상 아님.

**구조**: `Domain.h`(계약 상당), `Snapshot.h`(RootSnapshot 값 객체), `SnapshotLoader.{h,cpp}`(파일 I/O·파싱 전담, Repository 아님 — `ReadOutcome{Success,NotFound,ParseError,UnsupportedSchema}` 자체 어휘 사용), `FileSnapshotRepository.{h,cpp}`, `InMemoryRepository.{h,cpp}`, `ConsoleView.{h,cpp}`(FR-04, ISample/IOrderRepository만 의존).

**오류 어휘 사용 시점**: 6개 전부 **미접촉**. `Domain.h`에 `WriteOutcome`도 예외 3종도
선언되어 있지 않다. `InMemoryRepository.h`는 대신 `std::invalid_argument`(중복
sampleId/orderId), `std::out_of_range`(대상 없음)를 쓴다(주석에 명시). `SnapshotLoader`는
독자적인 `ReadOutcome` 열거형(Success/NotFound/ParseError/UnsupportedSchema)을 쓴다 — 6개
어휘와 겹치지 않는 별도 체계.

### 2.4 DummyDataGenerator

**계약 파일**: `Contract.h` — 미갱신(§1.4). `docs/CONTRACT.md` 사본 헤더는 v3, 코드는 v1 —
배포는 됐으나 반영되지 않았다(원인 미추정).

**구조**: `Contract.h`(계약 상당, `namespace contract`), `DummyDataGenerator.{h,cpp}`(생성 로직), `InMemoryRepository.{h,cpp}`(테스트/데모용 어댑터), `Tests.{h,cpp}`(FR-13 재현성 검증).

**오류 어휘 사용 시점**: 6개 전부 **미접촉**. `Contract.h`에 `WriteOutcome`도 예외 3종도 없음. `Add`/`Update`는 `void` 반환.

## 3. 축2 — 오류 어휘 6개 사용 시점 대조표

| 어휘 | ConsoleMVC | DataPersistence(기준) | DataMonitor | DummyDataGenerator | 일치 여부 |
|---|---|---|---|---|---|
| `WriteOutcome::Ok` | 사용(Add/Update 성공 반환) | 사용(Add/Update/Delete 성공 반환) | 미접촉 | 미접촉 | 불일치(2/4만 접촉) |
| `WriteOutcome::NotFound` | 사용(Update 대상 없음) | 사용(Update/Delete 대상 없음) | 미접촉 | 미접촉 | 불일치(2/4만 접촉) |
| `WriteOutcome::DuplicateKey` | 사용(Add 키 중복) | 사용(Add 키 중복) | 미접촉 | 미접촉 | 불일치(2/4만 접촉) |
| `StorageUnavailable` | 미접촉(인메모리라 발생 매체 없음) | 사용(파일 열기/쓰기/atomic replace 실패) | 미접촉 | 미접촉 | 불일치(1/4만 접촉) |
| `StorageCorrupted` | 미접촉 | 사용(파싱 실패, status 값 이상, 중복 레코드) | 미접촉 | 미접촉 | 불일치(1/4만 접촉) |
| `SchemaVersionMismatch` | 미접촉 | 사용(지원 안 하는 schemaVersion) | 미접촉 | 미접촉 | 불일치(1/4만 접촉) |

**불일치 건수: 6/6.** DataPersistence만 6개 전부 접촉(실제 파일 저장소 구현체를 갖고
있어서). ConsoleMVC는 [D] 범주만 접촉(인메모리라 [F] 범주 발생 매체가 없음). DataMonitor·
DummyDataGenerator는 6개 전부 미접촉(둘 다 v1 형태의 자체 어휘를 쓰거나 예외를 던지지
않는 인터페이스이기 때문 — §1.4/§1.5, §2.3/§2.4 참조).

## 4. 축3 — 이식 대장

"이식"은 ADR-I2에 따른 재작성 대상 판정이며 파일 복사가 아니다.

| 파일 | 소속 PoC | 판정 | 사유 | 귀속 v3 조항/ADR |
|---|---|---|---|---|
| `src/domain/Records.h` | ConsoleMVC | 이식(재작성 대상) | v3 완전 정합(§1.1) | CONTRACT §5, ADR-E1/E2 |
| `src/app/AppController.{h,cpp}` | ConsoleMVC | 이식(재작성 대상) | Controller, iostream 직접 호출 없이 IOutputPort만 통지(헤더 주석 확인) | CLAUDE.md §2 |
| `src/infra/console/ConsoleOutputAdapter.{h,cpp}` | ConsoleMVC | 이식(재작성 대상) | View-A, IOutputPort 구현 | - |
| `src/infra/console/PlainConsoleOutputAdapter.{h,cpp}` | ConsoleMVC | 이식(재작성 대상) | View-B, FR-05/FR-06 증명용(헤더 주석) | PRD FR-05/FR-06 |
| `src/infra/repository/InMemorySampleRepository.{h,cpp}` | ConsoleMVC | 이식(재작성 대상) | v3 WriteOutcome 정합 확인(§2.1) | - |
| `src/infra/repository/InMemoryOrderRepository.{h,cpp}` | ConsoleMVC | 이식(재작성 대상) | 상동 | - |
| `src/ports/IInputPort.h`, `IOutputPort.h`, `ISampleRepository.h`, `IOrderRepository.h` | ConsoleMVC | 이식(재작성 대상) | 포트 경계 선언, v3 시그니처 정합(§1.1) | - |
| `tests/ControllerScenarioTests.{h,cpp}`, `tests/fakes/FakeInputPort.{h,cpp}` | ConsoleMVC | 보류 | (a) 판정과 묶임, §5-(a) 참조 | - |
| `tests/fakes/RecordingOutputPort.{h,cpp}` | ConsoleMVC | 보류 | §5-(a) 사실 확인 대상, 최우선 항목 | - |
| `src/Contracts.h` | DataPersistence | 이식(재작성 대상) | `[[nodiscard]]` 누락 1건 = drift(§1.2), 나머지 정합 | CONTRACT §5.1/§5.2, ADR-E1 |
| `src/FileRepositories.{h,cpp}` | DataPersistence | 이식(재작성 대상) | 실제 영속 구현체, 6개 어휘 전부 접촉(§2.2) | ADR-E2 |
| `src/InMemoryRepositories.{h,cpp}` | DataPersistence | 이식(재작성 대상) | WriteOutcome 정합 | - |
| `src/JsonDocumentStore.{h,cpp}` | DataPersistence | 이식(재작성 대상) | 계약에 없는 내부 컴포넌트, 예외 재포장 경계(헤더 주석 명시) | ADR-E2 |
| `src/MiniJson.{h,cpp}` | DataPersistence | 이식(재작성 대상) | 계약에 없는 내부 유틸, "저장 방식은 저장소 자유"에 속함(헤더 주석) | CONTRACT §3 |
| `src/OrderStatusCodec.{h,cpp}` | DataPersistence | 보류 | (c) 판정과 묶임, §5-(c) 참조 | - |
| `Domain.h` | DataMonitor | 보류 | 미갱신(§1.5), v3 재작성 시 형태가 전부 바뀌어야 함 — 무엇이 남을지 이 조사만으로 확정 불가 | ADR-I2 |
| `Snapshot.h` | DataMonitor | 보류 | RootSnapshot이 Domain.h(미갱신)에 의존 — Domain.h 판정 미확정 상태에서 종속 판정 불가 | - |
| `SnapshotLoader.{h,cpp}` | DataMonitor | 보류 | 자체 `ReadOutcome` 어휘가 6개 어휘와 별개 체계(§2.3) — v3 편입 시 어휘 통합 여부 미확정 | - |
| `FileSnapshotRepository.{h,cpp}`, `InMemoryRepository.{h,cpp}`(DataMonitor) | DataMonitor | 보류 | Domain.h(v1) 시그니처 그대로 구현 — Domain.h 판정에 종속 | - |
| `ConsoleView.{h,cpp}` | DataMonitor | 보류 | ISample/IOrderRepository만 의존한다고 주석에 명시되나, 그 인터페이스 자체가 미갱신 | PRD FR-04 |
| `Contract.h` | DummyDataGenerator | 보류 | 미갱신(§1.4), v3 재작성 시 형태 전부 바뀜 | ADR-I2 |
| `DummyDataGenerator.{h,cpp}` | DummyDataGenerator | 보류 | (b) 채번 판정과 묶임, §5-(b) 참조 | - |
| `InMemoryRepository.{h,cpp}`(DummyDataGenerator) | DummyDataGenerator | 보류 | Contract.h(v1) 시그니처 그대로 구현, NextOrderId 구현체 포함(§5-(b)) | - |
| `Tests.{h,cpp}` | DummyDataGenerator | 보류 | FR-13 재현성 검증 스위트 — 대상 인터페이스(Contract.h) 판정 미확정 | ADR-I4(assert-count 방식과 관계) |

**건수 집계**: 이식(재작성 대상) 12건 / 잔류 0건 / 보류 12건.

### 보류 목록 상세 (지시된 4건 + 판정 불가로 보류된 항목)

**(a) `tests/ControllerScenarioTests.{h,cpp}`, `tests/fakes/RecordingOutputPort.{h,cpp}` — 최우선 확인 항목**

`RecordingOutputPort.h`(전문 인용):
```cpp
struct RecordedEvent {
    enum class Kind { MainMenuShown, SampleFormShown, SampleRegistered, Error, Exit };
    Kind kind{};
    SampleRecord sample{};
    std::string errorCode;
    std::string errorDetail;
};

// 테스트용 가짜 출력처: 화면에 아무것도 그리지 않고, 발생한 이벤트를
// 구조화된 값(RecordedEvent)으로만 기록한다. 테스트는 이 값을 단언한다 —
// 렌더링된 문자열을 비교하지 않는다.
class RecordingOutputPort : public IOutputPort {
public:
    void ShowMainMenu() override;
    void ShowSampleForm() override;
    void OnSampleRegistered(const SampleRecord& sample) override;
    void OnError(const std::string& code, const std::string& detail) override;
    void OnExit() override;

    std::vector<RecordedEvent> events;
};
```
`RecordingOutputPort.cpp`(전문): 5개 메서드 각각이 `events.push_back(RecordedEvent{Kind, sample|{}, code|"", detail|""})`만 수행 — 문자열 포맷팅/렌더링 코드가 전혀 없다.

**사실**: `RecordingOutputPort`는 렌더링된 화면 문자열을 비교하지 않는다. `IOutputPort`
인터페이스 호출(어느 메서드가 호출됐는지 — `Kind` — 와 그 인자 — `SampleRecord`/에러
코드·상세)을 구조화된 값으로 기록하고, 테스트는 그 구조화된 값을 단언한다. 판정하지 않음.

**(b) DummyDataGenerator 채번**

`DummyDataGenerator.cpp:99`: `order.orderId = orderRepo.NextOrderId();` — 생성기 함수
자체는 `orderRepo.NextOrderId()`를 호출할 뿐이다. 실제 계산 로직은
`InMemoryRepository.cpp:52-56`:
```cpp
int64_t InMemoryOrderRepository::NextOrderId() const {
    int64_t maxId = 0;
    for (const auto& r : records_) maxId = std::max(maxId, r.orderId);
    return maxId + 1;
}
```
즉 "FindAll 최대값+1" 계산은 생성기(`DummyDataGenerator.cpp`) 안이 아니라 Repository
구현체(`InMemoryRepository.cpp`)의 `NextOrderId()` 메서드 본문 안에 있다. `Contract.h:40`이
`IOrderRepository`에 `NextOrderId()`를 인터페이스 메서드로 선언하고 있다. 고치지 않았고
대안도 적지 않는다.

**(c) OrderStatusCodec**

`OrderStatusCodec.h`(전문 주석): "계약에 없는 내부 유틸리티. CONTRACT.md v3 §2 매핑
표를 그대로 코드로 옮긴 것뿐이며, wire format(§3, 문자열)과 in-memory
`OrderRecord::status`(§5, `OrderStatus` enum) 사이의 변환을 맡는다." `ToWireString`/
`FromWireString`은 `docs/CONTRACT.md:63-67`(§3 wire format 매핑표, `OrderStatus::Reserved` ↔
`"RESERVED"` 등 5쌍)과 문자열이 정확히 일치한다(RESERVED/REJECTED/PRODUCING/CONFIRMED/RELEASE).

**사실**: 이것은 v3 계약 자체가 정의한 "현재" wire format 직렬화/역직렬화이며, v1 문자열
데이터를 읽기 위한 스키마 마이그레이션 변환이 아니다. CONTRACT.md §3이 이 매핑을 v3의
일부로 명문화하고 있다.

**(d) main.cpp 3종 / 각 repo README·CLAUDE.md·docs**

본 repo 루트에는 `main.cpp`(7줄), `SampleOrderSystem.vcxproj`, `CLAUDE.md`가 이미 존재한다.
4개 PoC 전부 루트에 `main.cpp`, `CLAUDE.md`, `README.md`가 있고, `docs/PRD.md`(4개 전부),
`docs/CONTRACT.md`(4개 전부), `docs/DESIGN.md`(DataPersistence/DataMonitor/
DummyDataGenerator 3개)가 있다.

| 파일 | ConsoleMVC | DataPersistence | DataMonitor | DummyDataGenerator |
|---|---|---|---|---|
| `main.cpp` 줄 수 | 42 | 111 | 75 | 82 |
| `CLAUDE.md` | 있음 | 있음 | 있음 | 있음 |
| `README.md` | 있음 | 있음 | 있음 | 있음 |
| `docs/PRD.md` | 있음 | 있음 | 있음 | 있음 |
| `docs/CONTRACT.md` | 있음(v3 헤더) | 있음(v3 헤더) | 있음(v3 헤더) | 있음(v3 헤더) |
| `docs/DESIGN.md` | 없음 | 있음 | 있음 | 있음 |

**충돌 여부**: 본 repo 루트의 `main.cpp`(7줄), `CLAUDE.md`가 4개 PoC 전부의 동일 이름
파일과 경로·이름이 겹친다. 본 repo `docs/CONTRACT.md`(v3 원본)도 4개 PoC 사본과 경로가
겹친다. 판정하지 않음(삭제·덮어쓰기·병합 금지 지시에 따라 사실만 기록).

## 5. 축4 — 덮어쓰기 충돌

본 repo에 `src` 디렉터리는 없다. 코드는 루트 `main.cpp`(7줄)와
`SampleOrderSystem.vcxproj`뿐이다. 이 둘에 대해서만 충돌을 본다.

| 목적지 경로 | 기존 파일 존재 | 이식 후보 | 충돌 |
|---|---|---|---|
| `main.cpp`(루트) | 있음(7줄, 스켈레톤) | ConsoleMVC/DataPersistence/DataMonitor/DummyDataGenerator 각 `main.cpp`(42/111/75/82줄) | 있음 — 이름·경로 동일, 내용 상이 |
| `SampleOrderSystem.vcxproj` | 있음 | (PoC 쪽에 대응 파일 없음 — 각 PoC는 자체 프로젝트 파일 보유, 이름 다름) | 없음 |

`src` 디렉터리 자체가 없으므로 `src/*` 경로에 대한 충돌은 성립하지 않는다(목적지 부재).

## 6. 축5 — 기계적 검증 수치

| 항목 | ConsoleMVC | DataPersistence | DataMonitor | DummyDataGenerator | 본 repo |
|---|---|---|---|---|---|
| `Exists(` 건수 | 0 | 0 | 6 (`Domain.h:24`, `FileSnapshotRepository.{h:15,cpp:15}`, `InMemoryRepository.{h:11,cpp:14,19}`) | 5 (`Contract.h:29`, `DummyDataGenerator.cpp:47,103`, `InMemoryRepository.{h:18,cpp:19}`) | 0 |
| `void Add(`/`void Update(` 건수 | 0 | 0 | 8 (`Domain.h:25,26,36,37`, `FileSnapshotRepository.h:16,17,32,33`) — `InMemoryRepository.h:12,13,25,26`은 시그니처만 `override`, 별도 집계 시 총 12 | 6 (`Contract.h:30,31,41,42`, `InMemoryRepository.h:19,20,32,33` 별도 집계 시 총 8) | 0 |
| `NextOrderId` 건수 | 0 | 0(주석 1건: "의도적으로 없다") | 4 (`Domain.h:35`, `FileSnapshotRepository.{h:31,cpp:45}`, `InMemoryRepository.{h:24,cpp:51}`) | 3 (`Contract.h:40`, `DummyDataGenerator.cpp:99`, `InMemoryRepository.{h:31,cpp:52}`) | 0 |
| `OrderRecord.status` 실제 타입 | `OrderStatus`(enum) | `OrderStatus`(enum) | `std::string`(`Domain.h:15`) | `std::string`(`Contract.h:20`) | - |
| `FindByStatus` 인자 타입 | `OrderStatus`(열거형) | `OrderStatus`(열거형) | `const std::string&` | `const std::string&` | - |
| `FindBySampleId` 존재 | 있음(`IOrderRepository.h:15`) | 있음(`Contracts.h:81`) | 없음 | 없음 | - |
| `ISampleRepository::Delete` 존재 | 있음(`ISampleRepository.h:15`) | 있음(`Contracts.h:67`) | 없음 | 없음 | - |
| `WriteOutcome` 정의 건수/위치 | 1 (`Records.h:9`) | 1 (`Contracts.h:34`) | 0 | 0 | 0 |
| `[[nodiscard]]` 건수/위치 | 1 (`Records.h:9`) | 0 | 0 | 0 | 0 |
| 예외 3종(`StorageUnavailable`/`StorageCorrupted`/`SchemaVersionMismatch`) 정의 건수 | 3 (`Records.h:11,15,19`) | 3 (`Contracts.h:38,43,48`) | 0 | 0 | 0 |

**`WriteOutcome` 선언 개별 판정** (목표: 선언된 곳마다 `[[nodiscard]]` 존재):
- ConsoleMVC `Records.h:9` — `[[nodiscard]]` 있음. 충족.
- DataPersistence `Contracts.h:34` — `[[nodiscard]]` 없음. **미달**(§1.2 drift).
- DataMonitor, DummyDataGenerator — `WriteOutcome` 선언 자체가 없음(해당 없음).

0이 아닌 항목(Exists/void Add·Update/NextOrderId 잔존, status 타입 불일치, `[[nodiscard]]`
누락 등)은 그대로 위 표에 남겼다. 고치지 않았다.

## 7. 커밋 확인

`git add docs/BACKPORT-v3.md` 실행 후 `git show --stat HEAD`로 파일 1개 확인 예정(본 보고
직후 진행).
