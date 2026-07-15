# PRD ↔ CONTRACT v3 층위 정합 조사 (PD 계열, 조사 전용)

> 이 문서는 [CC-4.1] 지시에 따른 조사 산출물이다. `docs/PRD.md`, `docs/CONTRACT.md`(v3, 동결)를 포함해 어떤 문서도 이 조사 과정에서 수정하지 않았다.
> 진리 기준: `docs/SPEC.md` → `docs/DECISIONS.md`(ADR-Q/R/C/E 계열) → `docs/PRD.md` → `docs/CONTRACT.md` 순의 상류 우선.
> 이 조사에서 발견되는 항목의 식별자는 `PD-##`이며, CONTRACT 계약 공백(B 계열)과는 별개다.

---

## 0. 조사 방법

`docs/PRD.md` 전체(FR-01~FR-33, 도메인 모델 §2, 상태 전이 §3, 계산규칙 §5, 비범위 §6, OPEN ITEMS)를 아래 검색어로 grep한 뒤, 매칭된 모든 위치를 축 A(층위 판정)·축 B(생존 판정)로 수기 대조했다.

```
NextOrderId, Exists, WriteOutcome, OrderStatus, FindByStatus, FindBySampleId,
Add, Update, Delete, Repository, void, string status, ORD-,
schemaVersion, yieldNumerator, YIELD_DENOMINATOR, StorageUnavailable,
StorageCorrupted, SchemaVersionMismatch, CreatedAt, createdAt, decimal,
atomic rename, File.Replace, Named Mutex, int64_t, std::, optional<, vector<
```

**결과**: `NextOrderId, Exists, WriteOutcome, OrderStatus(열거형 이름 자체), FindByStatus, FindBySampleId, Repository, void, string status, ORD-, schemaVersion, yieldNumerator, YIELD_DENOMINATOR, StorageUnavailable, StorageCorrupted, SchemaVersionMismatch, int64_t, std::, optional<, vector<` — 이 목록 전체가 PRD.md에서 **0건** 매칭. `Add/Update/Delete`도 영문 식별자 형태로는 0건(모두 "등록/수정/삭제" 등 한글 서술).

매칭이 있었던 것은 `decimal`(FR-26), `atomic rename`/`File.Replace`(FR-30), `Named Mutex`(FR-24), `CreatedAt`(FR-32) 4곳뿐이다. 이하 이 4곳을 축 A/축 B로 판정한다.

---

## 1. 대장 (PD 계열)

| PD-# | PRD 위치(FR-##/줄) | PRD 현재 서술 | v3 근거(§/ADR) | 축A | 축B | 상류 근거(SPEC/ADR 조항) | 개정 필요 여부 |
|---|---|---|---|---|---|---|---|
| PD-01 | FR-26 / PRD.md:404 | "시스템은 수율 및 이를 사용한 실생산량/재고 반영량 계산에 부동소수점(`double`) 대신 고정소수점(`decimal`) 연산을 사용해야 한다." | CONTRACT.md §3("`yieldNumerator`: 실효값 = `yieldNumerator`/`YIELD_DENOMINATOR`(정수 연산 경로)"), §8 계산규칙 | 기술수단누출 | 유효-표현만낡음 | DECISIONS.md R4("채택: 선택안 A(정수 스케일 연산)... C++20에는 표준 `decimal` 타입이 없으므로... 선택안 B는 이 스택에 그대로 적용 불가") | 필요 |

**PD-01 판정 근거**:
- 축A(기술수단누출): FR-26은 "무엇을 만족해야 하는가"(부동소수점 오차 없는 정밀 계산)를 넘어 "어떤 자료형/연산 방식을 쓸 것인가"(`decimal`)까지 지목한다. 자료형·연산 방식 선택은 CONTRACT §1.2/§3/§8(Sample.Yield의 내부 표현)의 몫이다.
- 축B(유효-표현만낡음): 요구(수율 계산의 부동소수점 오차 배제)는 살아있고 CONTRACT v3에 그대로 구현되어 있다(`yieldNumerator`/`YIELD_DENOMINATOR` 정수 스케일). 그러나 PRD가 지목한 구체적 수단인 "`decimal`"은 채택되지 않았다 — DECISIONS.md R4 자신이 "C++20에는 `decimal` 표준 타입이 없어 이 스택에 직접 적용 불가"라고 명시적으로 기각한 선택지다. 즉 PRD가 지목한 기술 수단은 애초에 이 스택에서 채택 불가능했던 것으로 확정되었고, 실제로는 다른 수단(정수 분자/분모 스케일)으로 대체되었다.
- 이 사례는 발단이 된 FR-06/`NextOrderId` 사례와 "요구는 살아있는데 지목한 수단이 사라짐/불가능함이 확정됨"이라는 동일한 패턴을 보이는 유일한 확인 사례다.

**요구소멸(축B) 판정 행: 없음.** 대장 전체에서 축B가 "요구소멸"로 판정된 행은 0건이다.

---

## 2. 검토했으나 대장에서 제외한 후보 (참고, PD 번호 미부여)

grep에서 매칭되었지만 아래 사유로 "PRD가 CONTRACT의 몫인 인터페이스 심볼/메서드명/반환타입을 지목했다가 v3에서 어긋난" 이번 조사의 대상 패턴에 해당하지 않는다고 판단해 제외했다. 판단만 기록하며 개정 여부는 결정하지 않는다.

| 위치 | 매칭 문구 | 제외 사유 |
|---|---|---|
| FR-24 / PRD.md:392 | "(Named Mutex 또는 파일 배타 잠금 방식 권장)" | CONTRACT.md는 단일 인스턴스 가드(프로세스 락)를 전혀 다루지 않는다 — Repository/영속 데이터 계약과 무관한 영역이라 "v3에서 어긋남"을 판정할 CONTRACT 근거 자체가 없다. 또한 "권장"이라는 표현으로, 강제 규정이 아니라 예시 제시임이 문구상 명확하다. |
| FR-30 / PRD.md:436 | "원자적 교체(atomic rename/`File.Replace`) 방식을 사용하고" | CONTRACT.md §3이 동일 사안을 이미 "파일 형식·경로·원자적 교체 구현은 저장소 자유"로 명문화하면서 그 근거로 [PRD FR-30]을 직접 인용한다 — 즉 CONTRACT가 PRD의 이 요구를 받아 "구체적 기법은 자유"라고 화답한 관계이지, PRD가 CONTRACT의 확정 심볼을 지목했다가 어긋난 관계가 아니다. PRD의 병기(rename/File.Replace)도 상호배타적 API 지정이 아니라 "임시파일+교체" 패턴의 예시 나열로 읽힌다. |
| FR-32 / PRD.md:456 | "생성 시각(CreatedAt) 및 전이 시각이..." | CONTRACT.md §1.3의 `CreatedAt` 필드 자체가 그 근거로 [PRD FR-32]를 인용한다 — CONTRACT가 PRD를 근거로 이 필드명을 채택한 방향이며, PRD가 CONTRACT에서 사라진/바뀐 심볼을 사후에 지목한 사례가 아니다. `CreatedAt`은 v1부터 v3까지 변경 없이 유효하여, 이번 조사가 다루는 "v3 개정 이후 생존 여부" 축B 자체가 성립하지 않는다(애초에 아무것도 바뀌지 않았다). |

---

## 3. FR-06 / `NextOrderId` 원 제보 건에 대한 검증 결과

이번 조사를 촉발한 전제 — "PoC-2가 PRD.md의 FR-06이 사라진 `NextOrderId()`를 전제한다고 보고했다" — 를 grep과 원문 대조로 직접 검증했다.

- `docs/PRD.md` 전체에서 `NextOrderId` 문자열은 **0건**이다.
- FR-06 원문(PRD.md:158-164)은 "시스템은 시료ID, 고객명, 주문수를 입력받아 새 주문을 RESERVED 상태로 생성해야 한다... Then 자동증가 주문ID가 부여된 RESERVED 상태의 주문이 생성된다"로, 채번 방식이 "자동증가"라는 것만 서술하며 이를 수행하는 메서드/주체(Repository의 `NextOrderId`인지 Model의 `FindAll()+1`인지)는 전혀 지목하지 않는다.
- `NextOrderId`가 실제로 등장하는 문서는 `docs/CONTRACT.md`(v1~v3 변천, ADR-E4로 v3에서 제거), `docs/DECISIONS.md`(ADR-E4), `docs/AUDIT-CONTRACT-v2.md`(#34~#37, #27) 뿐이다. 이들은 모두 CONTRACT 인터페이스 자체의 감사 이력이지 PRD 서술과는 별개다.

**결론**: PRD.md 원문 기준으로 FR-06은 축A "기술수단누출"에 해당하지 않는다 — 자동증가 채번이라는 요구만 말하고 수단을 지목하지 않으므로 정상이다. PoC-2가 보고한 "FR-06이 사라진 NextOrderId()를 전제한다"는 진술은 PRD.md 원문과 대조했을 때 근거가 확인되지 않았다. 이 진술이 어디에서 비롯되었는지(다른 PoC 저장소의 사본, 혹은 CONTRACT/AUDIT 문서와의 혼동 가능성)는 이번 조사 범위(PRD.md 원문 대조) 밖이라 판단을 유보하고 사실만 기록한다.

---

## 4. 총계

- 대장(PD 계열) 총 행수: **1건** (PD-01)
- 축A 판정별 건수: 기술수단누출 1건 / 요구사항(정상) — 대장에 포함되지 않은 나머지 PRD 전체(FR-01~25, 27~33, §2/§3/§5/§6/OPEN ITEMS)는 grep 매칭이 없어 축A 판정 대상 자체가 아니었음.
- 축B 판정별 건수: 유효-표현만낡음 1건 / 유효-수임자이동 0건 / **요구소멸 0건**
- 요구소멸 행 유무: **없음**

---

## 5. 별도 답변 (6절, 판정 아님 — 관찰만)

**(a) PRD가 인터페이스 심볼을 지목하지 않도록 하는 서술 규칙 필요 여부**
- 현재 `docs/PRD.md` 서문(1-6행)에는 "SPEC/DECISIONS만 근거로 작성, 근거 없는 요구사항 미포함" 규칙만 있고, "인터페이스 심볼/메서드명/반환타입을 지목하지 않는다"는 규칙은 PRD.md에도 CLAUDE.md에도 없다.
- `CLAUDE.md` §2(아키텍처 규칙)는 Model/View/Controller 코드 계층 분리만 다루며 "문서 작성 시 계층 혼입 금지" 규칙은 없다.
- PD-01(FR-26의 `decimal` 지목)이 이 규칙 부재 상태에서 실제로 발생한 사례다. 규칙이 들어갈 자리 후보는 두 곳 다 근거가 있다: PRD.md 서문(문서 자체의 작성 원칙이므로)과 CLAUDE.md(이미 여러 문서·커밋 작성 규칙을 담고 있으므로) — 어느 쪽인지는 판정하지 않는다.

**(b) PRD 버전 표기 필요 여부**
- `docs/CONTRACT.md`는 최상단에 "버전: v3 | 최종 수정일: 2026-07-15" 헤더와 CHANGELOG(v1/v2/v3 변경 이력)를 갖는다.
- `docs/PRD.md`에는 버전 헤더도 CHANGELOG도 없다. grep 결과 "버전"이라는 단어가 등장하는 곳은 FR-31의 "본 버전의 범위에서 제외"(검색 기능의 범위 한정 문구) 한 곳뿐이며, 문서 자체의 버전 표기가 아니다.

**(c) CLAUDE.md의 "작업 시작 시 CONTRACT.md 버전 확인" 절차 부재가 이번 drift의 원인인지**
- `docs/CLAUDE.md` 전체를 grep한 결과, "버전"이라는 단어도 "CONTRACT"라는 단어도 **0건**이다.
- 즉 CLAUDE.md에는 "작업 시작 시 CONTRACT.md 버전을 확인하라"는 절차 자체가 현재 존재하지 않는다. CLAUDE.md가 지금 명시하는 것은 "작업 시작 전 반드시 `docs/PRD.md`와 `docs/DECISIONS.md`를 읽는다"뿐이며, `docs/CONTRACT.md`를 읽으라는 지시조차 없다.
- 이 사실(CONTRACT.md 열람/버전 확인 절차의 완전한 부재)을 그대로 기록한다.
