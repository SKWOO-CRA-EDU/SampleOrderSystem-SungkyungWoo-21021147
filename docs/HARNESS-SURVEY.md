# HARNESS-SURVEY — Phase 6 Harness 요구 조사

> 본 문서는 조사 전용 산출물이다. Harness를 만들지 않았고 코드를 작성하지 않았다.
> 기존 문서(SPEC.md/DECISIONS.md/PRD.md/CONTRACT.md/RISKS.md/SPEC_ANALYSIS.md)는 한 글자도 개정하지 않았다.
> 진리 기준: SPEC.md → DECISIONS.md → PRD.md → CONTRACT.md(v3). ADR 계열: Q1~Q16 / R1~R10 / C1~C3 / E1~E11 / I1~I4.

## 0. 전제 확인

- `docs/CONTRACT.md` 헤더 확인 결과: **v3, 최종 수정일 2026-07-15, 동결 선언 존재**(CONTRACT.md 3~6행). 명령서 전제와 일치. 계속 진행.
- **전제 불일치 1건 발견 (보고만 함, 수정 안 함)**: 명령서 §2 축1은 "v3 §8 의 불변식 2개(상태별 조회 합 == 전체 조회 크기 / 저장 status 는 열거값)"라고 지시했으나, 실제로 이 두 불변식은 CONTRACT.md **§6 "불변식"**(158~165행)에 있다. §8은 "계산 규칙"(Yield/ShortageQuantity 등 수식) 절이다. 이 불일치는 절 번호 오기로 보이며, 두 불변식 자체는 축1·축3에 포함했다.
- ADR-I4(테스트 프레임워크 미도입, assert-count 방식)는 DECISIONS.md 718~737행에서 확인. 프레임워크 검토는 하지 않았다.
- SPEC.md §4 검증절의 화면표기값(실생산량 206, 총생산시간 165)은 ADR-Q2에서 "SPEC 오기"로 기각되고 수식값(185, 148)이 채택되어 있음을 확인. 아래 모든 명제는 수식(SPEC §4, CONTRACT §8)에서 역산했으며 예시 화면 숫자를 픽스처 기대값으로 쓰지 않았다.

---

## 축1 — 검증 대상 명제 목록

각 행 = Harness가 지켜야 할 명제 1개. 근거 없는 행 없음.

| # | 명제 | 상류 근거 |
|---|---|---|
| 1 | Yield(실효값) = yieldNumerator / YIELD_DENOMINATOR(=10000) | [CONTRACT §8][PRD FR-26][ADR-R4] |
| 2 | ShortageQuantity = max(0, OrderQuantity − StockQuantity) | [CONTRACT §8][ADR-Q1] |
| 3 | ActualProductionQuantity = ceil(ShortageQuantity / Yield) | [SPEC §4][CONTRACT §8][PRD FR-17] |
| 4 | TotalProductionTime = AvgProductionTime × ActualProductionQuantity | [SPEC §4][CONTRACT §8][PRD FR-18][ADR-Q16] |
| 5 | 생산완료 시 재고반영량 = floor(ActualProductionQuantity × Yield) | [CONTRACT §8][PRD FR-19][ADR-Q13] |
| 6 | T1(RESERVED→CONFIRMED) 시 StockQuantity -= OrderQuantity | [CONTRACT §8][PRD FR-09][ADR-Q9] |
| 7 | T2(RESERVED→PRODUCING) 시 StockQuantity -= OrderQuantity | [CONTRACT §8][PRD FR-10][ADR-Q9] |
| 8 | T4(PRODUCING→CONFIRMED) 시 StockQuantity += floor(ActualProductionQuantity × Yield) | [CONTRACT §8][PRD FR-19][ADR-Q13] |
| 9 | T6(CONFIRMED→REJECTED) 시 StockQuantity += OrderQuantity | [CONTRACT §8][PRD FR-12][ADR-Q7] |
| 10 | 재고상태 판정: StockQuantity=0 → Depleted | [SPEC §4][CONTRACT §8][ADR-Q5] |
| 11 | 재고상태 판정: 0 < StockQuantity < OrderQuantity → Insufficient | [SPEC §4][CONTRACT §8][ADR-Q5] |
| 12 | 재고상태 판정: StockQuantity ≥ OrderQuantity → Sufficient | [SPEC §4][CONTRACT §8][ADR-Q5] |
| 13 | Yield ∈ (0,1], 등록 시점 강제 | [CONTRACT §1.2][ADR-Q3][ADR-Q4] |
| 14 | AvgProductionTime > 0, 등록 시점 강제 | [CONTRACT §1.2][ADR-Q12] |
| 15 | StockQuantity ≥ 0, 코드 레벨 불변식 | [CONTRACT §1.2][ADR-Q1][ADR-Q12] |
| 16 | 생산큐: 단일 생산라인, 한 번에 하나의 항목만 실행 상태 | [CONTRACT §1.4][ADR-Q11] |
| 17 | OrderId는 자동증가 정수(1,2,3,...)이며 Order의 유일한 키 | [CONTRACT §1.3][ADR-Q15][ADR-E5] |
| 18 | SampleId는 고유(등록 시 중복 거부) | [CONTRACT §1.2][PRD FR-01][ADR-C1] |
| 19 | 허용 전이는 T1~T6 6개뿐(§4 표) | [CONTRACT §4][PRD §3] |
| 20 | PRODUCING → REJECTED 전이 미지원 | [CONTRACT §4][ADR-Q6] |
| 21 | RELEASE → (전체) 전이 미지원, 종단 상태 | [CONTRACT §4][ADR-Q8] |
| 22 | REJECTED → (전체) 전이 미지원, 종단 상태 | [CONTRACT §4][ADR-Q8] |
| 23 | REJECTED/RELEASE 이후 §4 외 전이로 필드 변경 금지 | [CONTRACT §3][ADR-Q8] |
| 24 | **[v3 §6 불변식1]** 모든 OrderStatus 값에 대한 FindByStatus 결과 크기의 합 = FindAll 결과 크기 | [CONTRACT §6][ADR-E10] |
| 25 | **[v3 §6 불변식2]** 저장된 Order.status 값은 §2의 5개 열거값 중 하나 | [CONTRACT §6][ADR-E10] |
| 26 | 위 24/25 불변식 위반 시 StorageCorrupted로 취급 | [CONTRACT §6][ADR-E10] |
| 27 | Repository는 키 유일성/대상 존재/저장매체 상태만 검증(참조무결성·전이적법성·수량범위 등은 Model 책임) | [CONTRACT §5.4][ADR-E9] |
| 28 | Sample Delete는 참조하는 Order가 하나도 없을 때만 성공(상태 무관, REJECTED/RELEASE 이력 포함) | [CONTRACT §5.3][ADR-C1][ADR-C2][ADR-E8] |
| 29 | 저장 계층 예외는 6개 어휘(WriteOutcome 3값 + 예외 3종)로만 표면화되며 구현 라이브러리 예외가 새어나오면 결함 | [CONTRACT §5.2][ADR-E2] |
| 30 | 스키마 버전이 프로그램이 아는 값과 다르면 SchemaVersionMismatch를 던지고 종료(변환 안 함) | [CONTRACT §3][CONTRACT §9][ADR-E11] |
| 31 | 손상 레코드 1건 발견 시 저장소 전체를 StorageCorrupted로 간주(부분 스킵 금지) | [CONTRACT §9][ADR-E11] |
| 32 | Order.CreatedAt은 생성 시 기록되고 이후 불변 | [CONTRACT §1.3][PRD FR-32][ADR-R10] |
| 33 | IOrderRepository에는 Delete/Remove/NextOrderId 메서드가 존재하지 않음(의도적 설계) | [CONTRACT §7][ADR-C3][ADR-E4] |

총 **33개 명제**.

---

## 축2 — 스냅샷의 정의 (후보 비교만, 선택 안 함)

### 후보 A — 저장 파일 원본 바이트열 비교
- 무엇을 직렬화하는가: 저장 파일 전체 바이트열(디스크상의 실제 파일 내용)
- InMemory/파일 저장소 양쪽에 같은 Harness가 도는가: **아니오** — InMemory 구현체는 파일이 없으므로 이 정의 자체가 적용 불가
- 저장 방식에 결합되는가: **예** — 구체적 직렬화 포맷(JSON 들여쓰기, 키 순서, 부동소수점 문자열 표현 등)에 직접 결합됨. CONTRACT.md는 "파일 형식·경로·원자적 교체 구현은 저장소 자유"(§3, 79행)라고 명시하므로, 저장소 자유 영역을 Harness가 비교 대상으로 삼는 셈 — ADR-E3(저장 방식과 무관한 어휘 원칙)의 정신과 어긋나는 지점
- 비결정성 원천 누출: 배열 저장 순서, CreatedAt(ISO 8601 문자열) 값 자체, 부동소수점/정수 포맷팅 방식이 스냅샷에 그대로 노출됨

### 후보 B — Repository 반환값(FindAll 등) 구조 비교, 원본 순서 유지
- 무엇을 직렬화하는가: `ISampleRepository::FindAll()` / `IOrderRepository::FindAll()`이 반환하는 `vector<SampleRecord>`/`vector<OrderRecord>`의 값 목록(파일 포맷과 무관, 메모리 상 구조체 값)
- 양쪽에 같은 Harness가 도는가: **예** — FindAll()은 CONTRACT §7의 [R] 범주 메서드로 InMemory/파일 구현 모두에 존재하는 동일 관측점
- 저장 방식에 결합되는가: **아니오** — Repository 인터페이스 반환값만 사용, 파일 포맷을 참조하지 않음
- 비결정성 원천 누출: **있음** — FindAll()이 반환하는 vector의 순회 순서가 구현체마다 다를 수 있고(§7에 정렬 보장이 없음), 이 순서가 그대로 스냅샷 비교 결과에 새어 들어갈 수 있음. CreatedAt 필드가 포함되면 타임스탬프 자체도 비결정성 원천이 됨

### 후보 C — 키 기준 정규화(정렬) 후 필드 값 비교
- 무엇을 직렬화하는가: FindAll() 결과를 sampleId/orderId로 정렬한 뒤의 필드 값 목록
- 양쪽에 같은 Harness가 도는가: **예** — 후보 B와 동일하게 FindAll() 기반
- 저장 방식에 결합되는가: **아니오**
- 비결정성 원천 누출: 순회 순서 문제는 정렬로 제거되나, CreatedAt(타임스탬프) 필드를 비교 대상에 포함할지 여부는 이 후보 자체가 정하지 않음 — 포함하면 여전히 비결정성 원천이 남고, 제외하면 §6 CHANGELOG가 요구하는 "필드 전체" 비교 범위를 벗어남(이 트레이드오프는 후보의 정의에 내재)

### 후보 D — 불변식·파생 스칼라값만 비교(개수/합계)
- 무엇을 직렬화하는가: `FindAll().size()`, `FindByStatus(status).size()`(5회) 등 파생 스칼라
- 양쪽에 같은 Harness가 도는가: **예**
- 저장 방식에 결합되는가: **아니오**
- 비결정성 원천 누출: **없음**(개수 비교는 순서 무관) — 그러나 개별 필드 값(예: 특정 orderId의 status가 정확히 기대값인지)의 정합성은 이 후보로는 검증하지 못함. "저장 데이터 스냅샷 비교"라는 표현이 요구하는 범위보다 좁을 가능성

이상 4개 후보를 비교만 했다. 선택하지 않았다.

---

## 축3 — 관측 가능성 대응표

축1의 33개 명제를 CONTRACT §7의 11개 메서드(`ISampleRepository`: FindById/FindAll/Add/Update/Delete, `IOrderRepository`: FindById/FindAll/FindByStatus/FindBySampleId/Add/Update) 중 무엇으로 관측하는지 대응시킨다.

| # | 명제 | 관측 방법 | 비고 |
|---|---|---|---|
| 1 | Yield 계산식 | **관측불가** | Yield 실효값은 Repository 어떤 메서드도 반환하지 않는다. `SampleRecord.yieldNumerator`(원시 정수)만 FindById/FindAll로 관측 가능하며, 나눗셈(÷YIELD_DENOMINATOR)은 Model 계산이지 Repository 관측점이 아니다. |
| 2 | ShortageQuantity | **관측불가** | ProductionQueueItem은 CONTRACT §3 영속 스키마(SampleRecord/OrderRecord)에 대응 필드가 없다. 11개 메서드 중 어느 것도 이 값을 반환하지 않는다. |
| 3 | ActualProductionQuantity | **관측불가** | 2와 동일 이유. |
| 4 | TotalProductionTime | **관측불가** | 2와 동일 이유. |
| 5 | 생산완료 시 재고반영량 | **관측가능(간접)** | 직접 반환되진 않으나, T4 전후 `ISampleRepository::FindById(sampleId)`의 `stockQuantity` 필드 차분(diff)으로 반영량을 역산 관측 가능. |
| 6 | T1 재고 차감 | **관측가능** | T1 실행 전후 `FindById(sampleId).stockQuantity` 비교. |
| 7 | T2 재고 차감 | **관측가능** | 6과 동일 메서드. |
| 8 | T4 재고 증가 | 5와 동일(간접 diff). |
| 9 | T6 재고 복구 | **관측가능** | T6 전후 `FindById(sampleId).stockQuantity` 비교. |
| 10~12 | 재고상태 판정(Sufficient/Insufficient/Depleted) | **관측불가** | 이 판정 문자열/열거값은 CONTRACT §2에서 "Status 아님, 모니터링 전용"으로 별도 명시되며 저장 스키마 필드가 아니다. Repository 어떤 메서드도 이 값을 반환하지 않는다. `stockQuantity` 필드로 판정을 재계산할 수는 있으나 이는 Model 로직의 재구현이지 Repository 관측이 아니다. |
| 13 | Yield∈(0,1] 강제 | **관측가능(간접, 강제 여부는 불가)** | `FindById(sampleId).yieldNumerator`로 저장된 값의 범위(1..YIELD_DENOMINATOR)는 확인 가능. 그러나 "강제되는지"(즉 Add가 범위 밖 값을 거부하는지)는 §5.4/ADR-E9에 따라 Repository 책임이 아니므로, Repository 계약만으로는 강제 여부 자체를 관측할 수 없다 — Model 호출 결과(거부 메시지 등)로만 관측 가능. |
| 14 | AvgProductionTime>0 강제 | 13과 동일 구조. |
| 15 | StockQuantity≥0 | **관측가능(값만)** | FindById/FindAll 필드값. 강제 메커니즘 자체는 관측불가(13과 동일 이유). |
| 16 | 단일 생산라인 불변식 | **관측불가** | 생산큐/ProductionQueueItem이 영속 스키마 밖 개념. |
| 17 | OrderId 자동증가·유일 키 | **관측가능** | `IOrderRepository::Add`의 `WriteOutcome`(DuplicateKey 발생 여부) + `FindAll()`의 orderId 값 목록. |
| 18 | SampleId 고유 | **관측가능** | `ISampleRepository::Add`의 `WriteOutcome::DuplicateKey`. |
| 19 | 허용 전이 T1~T6뿐 | **관측가능(간접)** | `FindById(orderId)`의 전/후 status 비교로 전이 발생 자체는 관측 가능. 단 "그 전이가 §4에 있는 것인지"는 Repository가 검증하지 않으므로(§5.4) Model이 실제로 §4를 지켰는지는 Repository 관측만으로 증명 불가 — Model 호출 계약(반환값)까지 함께 봐야 함. |
| 20 | PRODUCING→REJECTED 미지원 | **관측불가(Repository 레벨)** | `IOrderRepository::Update`는 전이 적법성을 검증하지 않으므로(§5.4/E9) 이 거부 자체가 Repository 계약에 나타나지 않는다. 거부는 Model/Controller 반환값에서만 관측되며, 11개 메서드 중 이를 표현하는 것이 없다. |
| 21 | RELEASE 종단 | 20과 동일 이유로 **관측불가(Repository 레벨)**. |
| 22 | REJECTED 종단 | 20과 동일. |
| 23 | REJECTED/RELEASE 이후 필드 변경 금지 | **관측가능(간접)** | 종단 상태 도달 후 `FindById`로 필드값을 반복 조회해 불변 여부 확인 가능(단, "왜" 안 바뀌는지는 Model 책임). |
| 24 | FindByStatus 합 = FindAll 크기 | **관측가능** | `IOrderRepository::FindByStatus`(5회, 각 OrderStatus) + `FindAll()`을 직접 호출해 크기 비교. |
| 25 | 저장된 status가 열거값 중 하나 | **관측가능(간접)** | `FindAll()`이 반환하는 `OrderRecord.status`는 C++ `OrderStatus` enum 타입이므로 타입 시스템상 열거값 외 값 자체가 표현 불가(컴파일 타임 강제). 파일이 손상되어 다른 문자열이 저장돼 있으면, "FindAll이 그 값을 반환하는지"가 아니라 "로드 단계에서 StorageCorrupted를 던지는지"로 관측해야 한다. |
| 26 | 24/25 위반 시 StorageCorrupted | **관측가능** | 파일을 인위적으로 손상시킨 뒤 로드 시 예외 타입 확인. |
| 27 | Repository 검증 범위 한정 | **관측가능** | Add/Update/Delete의 WriteOutcome 값과, 참조무결성·전이적법성 위반 입력을 Repository에 직접 넣었을 때 거부하지 않는지 확인. |
| 28 | Sample Delete 참조무결성 가드 | **관측가능(간접)** | `IOrderRepository::FindBySampleId(sampleId)` 결과(비었는지) + `ISampleRepository::Delete`의 WriteOutcome으로 결과 상태는 확인 가능. 단 "Delete 호출 전에 FindBySampleId를 먼저 호출했는지"라는 Model 내부 호출 순서 자체는 Repository 인터페이스만으로 관측 불가 — 결과(참조 있는데 삭제됐는지)만 관측 가능. |
| 29 | 6개 오류 어휘 폐쇄 | **관측가능** | 저장 매체 실패를 유발한 뒤 catch되는 예외의 정확한 타입 검사(6개 어휘 외 타입이면 위반). |
| 30 | SchemaVersionMismatch | **관측가능** | schemaVersion 필드를 변조한 뒤 로드 시 예외. |
| 31 | 손상 레코드 시 전체 StorageCorrupted | **관측가능** | 저장 파일을 부분 손상시킨 뒤 로드 시 예외. |
| 32 | CreatedAt 불변 | **관측가능** | `FindById(orderId).createdAt`을 여러 시점에 조회해 값이 바뀌지 않는지 확인. |
| 33 | IOrderRepository에 Delete/NextOrderId 없음 | **관측가능(계약 자체)** | CONTRACT.md §7 코드 선언 자체를 정적으로 확인(런타임 관측이 아니라 인터페이스 선언 검사). |

**관측불가 집계**: #1, #2, #3, #4, #10, #11, #12, #16, #20, #21, #22 — **11건**.

---

## 축4 — 가상 Clock 주입 지점 (후보 열거만, 선택 안 함)

CONTRACT.md §7에는 Clock/시각 관련 메서드나 파라미터가 전혀 없다. `OrderRecord.createdAt`은 단순 `string` 필드이며, 그 값을 만드는 시각 소스가 무엇인지는 계약이 규정하지 않는다.

### 후보 1 — Model(도메인 서비스) 내부에 Clock 추상화 주입
- 동결된 11개 인터페이스를 건드리는가: **아니오**
- 계층: **Model** — Order 생성/상태 전이 로직이 CreatedAt을 채울 때 내부적으로 Clock 인터페이스를 호출

### 후보 2 — 합성 루트(composition root, main)에서 Clock 구현체 주입
- 건드리는가: **아니오**
- 계층: **합성 루트** — main()이 Real Clock 또는 Fake Clock을 선택해 Model 생성자에 전달

### 후보 3 — Controller가 시각을 얻어 Model 호출 인자로 전달
- 건드리는가: **아니오**
- 계층: **Controller** — Model의 주문 생성/전이 API가 CreatedAt(또는 현재시각)을 파라미터로 받는 구조라면 Controller가 Clock을 호출해 값을 넘겨줌. (이 경우 Model API 시그니처가 시각을 파라미터로 받는지 여부는 CONTRACT 밖의 별도 설계 문제이며 이 조사가 결정하지 않는다.)

### 후보 4 — R6/FR-28 생산완료 트리거의 "경과시간" 판정용 시뮬레이션 시계
- 건드리는가: **아니오**(계약에 무관)
- 계층: 해당사항 없음 — DECISIONS.md R6(334~353행)는 채택안 A(수동 "생산완료 처리" 명령, 경과시간 계산 없음)를 확정했으므로, 현재 결정상 이 지점은 사용되지 않는다. 사실만 기록: 만약 향후 "틱 진행" 방식(R6 선택안 B)으로 바뀐다면 이 지점이 필요해지나, 이는 R6 재결정을 전제하므로 이 조사의 범위 밖이다.

이상 4개 후보를 열거만 했다. 선택하지 않았다.

---

## 축5 — 실패 주입

계약(CONTRACT §5.2/§7)이 수단을 제공하는지 여부만 사실로 기록.

| 어휘 | 무엇을 해야 이 신호가 나오는가 |
|---|---|
| `WriteOutcome::NotFound` | `ISampleRepository::Update`/`Delete` 또는 `IOrderRepository::Update`에 존재하지 않는 키(sampleId/orderId)를 전달한다. CONTRACT §7 주석에 "대상 없음 시 NotFound"로 명시. **수단 있음.** |
| `WriteOutcome::DuplicateKey` | `ISampleRepository::Add` 또는 `IOrderRepository::Add`에 이미 존재하는 키를 가진 레코드를 전달한다. CONTRACT §7 주석에 "키 중복 시 DuplicateKey"로 명시. **수단 있음.** |
| `StorageUnavailable` | CONTRACT §5.2는 "저장소를 읽거나 쓸 수 없음"이라고 의미만 정의할 뿐, 무엇을 하면 이 상태가 되는지(파일 잠금, 권한 제거, 디스크 마운트 해제 등)는 규정하지 않는다. §3은 "파일 형식·경로·원자적 교체 구현은 저장소 자유"라고 명시하므로, 저장 매체 접근을 실패시키는 구체적 방법은 구현체마다 다르다. **수단없음**(계약 레벨). |
| `StorageCorrupted` | CONTRACT §5.2는 "읽었으나 해석 불가 또는 불변식 위반"이라고 의미만 정의한다. §6의 두 불변식 위반 시 이 예외로 취급해야 한다는 **의무**는 규정되어 있으나, Harness가 계약만으로(구현에 의존하지 않고) 저장소를 손상 상태로 만드는 방법은 §3이 "파일 형식·경로는 저장소 자유"라고 선언한 이상 제공되지 않는다. **수단없음**(계약 레벨). |
| `SchemaVersionMismatch` | `schemaVersion` 필드가 §3에 정의되어 있으나("이름/타입/초깃값은 [결정-리서치]"), 이 필드가 저장되는 파일의 경로·포맷은 저장소 자유이므로 Harness가 계약만으로 이 필드를 변조할 방법이 없다. **수단없음**(계약 레벨). |

**수단없음 집계**: 3건(StorageUnavailable, StorageCorrupted, SchemaVersionMismatch). WriteOutcome 2종은 수단 있음.

---

## 축6 — 화면 문자열 비교 금지 하의 ConsoleMVC 검증

현재 계약(Repository 인터페이스 11개만 동결)과 CLAUDE.md §2 구조(View/Controller 인터페이스는 CONTRACT.md에 정의되지 않음, PRD/CONTRACT 어디에도 View/Controller의 관측 가능한 계약이 없음) 하에서 가능한 관측점만 기술한다.

- **관측 가능한 것**: Controller가 어떤 명령(예: 주문승인)을 실행한 뒤, Repository 상태(축2의 스냅샷 후보)가 기대한 대로 변했는지는 확인할 수 있다. 이는 "Model 로직이 실행되어 저장소에 반영되었다"는 것을 보여준다.
- **관측 불가능한 것**: "Controller → View 관통 자체"(즉 Controller가 실제로 View를 호출해 표시를 시도했는지, 몇 번 호출했는지, 어떤 인자로 호출했는지)를 확인할 동결된 계약상의 지점이 없다. CONTRACT.md는 Repository만 동결했고 View/Controller 인터페이스는 계약 범위 밖이다. View 호출 여부를 관측하려면 View 자리에 대체 가능한 관측 가능한 무언가(예: 테스트 전용 View 구현체)가 필요한데, 그런 구조가 계약에 없다.
- 결론: 화면 문자열 비교를 금지한 상태에서 "ConsoleMVC 관통"(Controller→View 경로 자체) 검증에 쓸 수 있는 현재 계약상의 관측점은 **없음**. Repository 상태 diff는 Model 계층까지의 관통만 보여주며 View 관통은 보여주지 못한다.

---

## 축7 — 픽스처 공급 요건

PoC-4(시드 고정 생성기, 문서상 명칭은 DummyDataGenerator — CONTRACT.md CHANGELOG/§ 전반에서 이 이름으로만 지칭됨)의 구현을 가정하지 않고, Harness가 요구하는 바를 **요건으로만** 적는다. 채번(orderId)은 Model 책임(ADR-E4)이므로 생성기가 orderId를 만든다고 가정하지 않는다.

1. **Sample 입력값 집합**: `sampleId`(문자열, 고유해야 함), `name`, `avgProductionTime`(>0), `yieldNumerator`(1..YIELD_DENOMINATOR 범위 정수) — CONTRACT §7 `SampleRecord` 필드에 대응하는 값을, Harness가 재현 가능한 방식(시드 고정)으로 공급받아야 한다.
2. **Order 입력값 집합**: `sampleId`(참조 대상 — 사전에 존재하는 Sample이거나, 참조무결성 위반 테스트를 위해 의도적으로 존재하지 않는 값), `customerName`, `orderQuantity`(>0). **`orderId`는 포함하지 않아야 한다**(Model이 채번). `CreatedAt`도 생성기가 임의로 채우지 않아야 한다 — 이는 축4의 Clock 문제이지 Fixture 문제가 아니다(생성기가 CreatedAt을 직접 결정하면 Model/Clock 계층의 책임을 침범한다).
3. **재현성(결정성) 요건**: 동일 시드값을 넣으면 Sample/Order 입력값의 시퀀스가 매 실행 동일해야 한다. 이는 SPEC/CONTRACT에 근거가 없는 순수 Harness 요건이며, ADR-I4(assert-count 방식, 프레임워크 없이 자체 집계)가 전제하는 "검증 결과가 재현 가능해야 한다"는 것과 정합적이어야 한다는 사실만 적는다 — 이 요건 자체를 어떻게 만족시킬지는 생성기의 구현 문제.
4. **경계값 커버리지 요건**: ADR-R4(부동소수점 재현성) 검증(축1 #3)을 위해 "ShortageQuantity/Yield가 정확히 나누어떨어지는 조합"(예: 부족분 90, 수율 0.9)을 만들어낼 수 있는 파라미터화가 필요하다 — 생성기가 StockQuantity/OrderQuantity/yieldNumerator를 개별 지정 가능해야 이런 경계 조합을 재현할 수 있다.
5. **상태 다양성 요건**: 축1 #24(FindByStatus 합=FindAll 크기) 검증을 위해 5개 OrderStatus 값 전부를 가진 Order가 최소 1건씩 필요하다. 단, Order.Status는 원칙적으로 Model의 상태 전이(T1~T6)를 거쳐야 도달하는 값이며, `IOrderRepository::Add`가 임의 상태의 `OrderRecord`(status 포함)를 그대로 받아들이는지(§5.4상 Repository는 전이 적법성을 검증하지 않으므로 기술적으로는 가능해 보임)와 그것이 정당한 Fixture 시딩 방식인지는 이 조사에서 판단하지 않는다 — 사실만: 계약은 Add가 status 필드를 검증하지 않는다고 명시하므로(축1 #27), 임의 상태 주입이 계약 위반은 아니다.
6. **참조무결성 위반 케이스 요건**: 축1 #28(참조무결성 가드) 검증을 위해, 존재하지 않는 `sampleId`를 참조하는 `OrderRecord`를 의도적으로 만들 수 있는 파라미터화가 필요하다.

---

## 4. 종합 보고

- **축1 명제 수**: 33개.
- **축3 관측불가 건수**: 11건(#1 Yield계산, #2 ShortageQuantity, #3 ActualProductionQuantity, #4 TotalProductionTime, #10~12 재고상태 판정 3종, #16 단일생산라인 불변식, #20 PRODUCING→REJECTED 거부, #21 RELEASE 종단 거부, #22 REJECTED 종단 거부 — Repository 레벨 관측 기준).
- **축5 수단없음 건수**: 3건(StorageUnavailable, StorageCorrupted, SchemaVersionMismatch). WriteOutcome 2종(NotFound, DuplicateKey)은 수단 있음.
- **축2 후보 개수**: 4개(A 파일 바이트열 / B FindAll 원순서 / C 정렬 정규화 / D 파생 스칼라만). 우열 판정 없음.
- **축4 후보 개수**: 4개(Model 내부 / 합성 루트 / Controller 파라미터 / R6 시뮬레이션 시계 — 단, 4번째는 현재 R6 결정(수동 트리거)상 미사용). 우열 판정 없음. 4개 후보 전부 동결된 11개 인터페이스를 건드리지 않음(계층은 Model/합성 루트/Controller).
- **축6 결과**: 현재 계약·구조에서 화면 문자열 비교 없이 Controller→View 관통 자체를 검증할 관측점 **없음**. Repository 상태 diff는 Model 계층 관통까지만 보여줌.

다음 단계·이식 계획·프레임워크 추천은 기술하지 않았다.
