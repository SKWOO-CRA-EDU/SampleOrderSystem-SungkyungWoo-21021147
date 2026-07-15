# CLAUDE.md

반도체 시료 생산 주문관리 시스템(S-Semi) — C++20 MVC 콘솔 앱. 재고·주문·생산큐 상태를 정합성 있게 관리하는 것이 핵심.

**작업 시작 전 반드시 `docs/PRD.md`와 `docs/DECISIONS.md`를 읽는다.** 요구사항 근거(FR-ID, ADR-Qn)를 확인하지 않고 기능을 구현하지 않는다. `docs/SPEC.md`, `docs/RISKS.md`, `docs/SPEC_ANALYSIS.md`는 근거가 필요할 때 보조로 참조한다.

---

## 1. 기술 스택 / 빌드 / 테스트 / harness

- 언어: C++20 (MSVC, `LanguageStandard=stdcpp20`)
- 프로젝트 형식: Visual Studio `.vcxproj` (`SampleOrderSystem.vcxproj`), Win32/x64, Debug/Release
- 진입점: `main.cpp` (현재 스켈레톤 상태 — MVC 디렉터리 구조는 아직 구현되지 않음)

빌드(명령줄, Developer Command Prompt 또는 `vswhere`로 찾은 `msbuild.exe` 필요):
```
msbuild SampleOrderSystem.vcxproj /p:Configuration=Debug /p:Platform=x64
```
릴리스 빌드는 `Configuration=Release`로 교체.

테스트/harness:
- 현재 저장소에는 테스트 프로젝트/harness가 존재하지 않는다. MVC 골격을 만들 때 테스트 프레임워크(예: Catch2, doctest) 도입 여부와 테스트 프로젝트 구조를 **먼저 사용자에게 확인**한 뒤 구성한다 — 임의로 프레임워크를 도입하지 않는다.
- 테스트/harness 구성이 결정되면 이 섹션에 실행 명령을 반드시 추가한다.
- `/verify` 스킬로 변경사항이 실제로 동작하는지 항상 확인한다(콘솔 앱이므로 최소한 실행하여 핵심 플로우를 눈으로 확인).

---

## 2. 아키텍처 규칙 (MVC)

- **Model**은 View와 Controller를 알지 못한다. Model 헤더/구현에 `<iostream>`, View 타입, Controller 타입을 포함하지 않는다.
- **Controller**는 `iostream`(또는 `printf` 등 콘솔 I/O)을 직접 호출하지 않는다. 입출력은 항상 View를 통한다. Controller는 Model 조작과 View 호출을 조율하는 역할만 한다.
- **View**는 도메인 로직(재고 계산, 상태 전이 조건, 수율/실생산량 계산 등)을 갖지 않는다. View는 Model이 계산한 값을 표시 형식으로만 변환한다.
- **영속성**은 Repository 인터페이스 뒤에 숨긴다. Model/Controller는 구체적인 저장 방식(파일, JSON 등)에 직접 의존하지 않고 Repository 인터페이스에만 의존한다.

---

## 3. 도메인 용어 사전 (한국어 개념 ↔ 영문 식별자)

| 한국어 | 영문 식별자 |
|---|---|
| 시료 | Sample |
| 시료ID | SampleId |
| 이름 | Name |
| 평균생산시간 | AvgProductionTime |
| 수율 | Yield |
| 재고수량 | StockQuantity |
| 주문 | Order |
| 주문ID | OrderId |
| 고객명 | CustomerName |
| 주문수 | OrderQuantity |
| 상태(주문) | Status (RESERVED / REJECTED / PRODUCING / CONFIRMED / RELEASE) |
| 생산큐 | ProductionQueue |
| 생산큐 항목 | ProductionQueueItem |
| 부족분 | ShortageQuantity |
| 실생산량 | ActualProductionQuantity |
| 총생산시간 | TotalProductionTime |
| 여유 / 부족 / 고갈 | Sufficient / Insufficient / Depleted |
| 시료등록 | RegisterSample |
| 시료조회 | ListSamples |
| 시료검색 | SearchSamples |
| 시료예약(주문 생성) | ReserveOrder |
| 접수된 주문목록 | PendingOrderList |
| 주문승인 | ApproveOrder |
| 주문거절 | RejectOrder |
| CONFIRMED 주문 취소 | CancelConfirmedOrder |
| 출고처리 | ReleaseOrder |
| 생산완료 처리 | CompleteProduction |
| 생산현황표기 | ProductionStatusDisplay |
| 대기주문확인 | CheckQueuedOrders |
| 주문량확인 | CheckOrderVolume |
| 재고량확인 | CheckStockLevel |

용어를 벗어난 이름(예: `Item`, `Qty`, `Stat`)을 새로 만들지 말고 위 표를 그대로 재사용한다. 표에 없는 새 개념이 필요하면 PRD 근거를 먼저 찾고, 없으면 사용자에게 질문한다.

---

## 4. 커밋 규칙

- [Conventional Commits](https://www.conventionalcommits.org/) 형식 사용: `feat:`, `fix:`, `refactor:`, `test:`, `docs:`, `chore:` 등.
- 1 커밋 = 1 논리적 변경. 여러 FR을 한 커밋에 섞지 않는다.
- 커밋 메시지 본문 또는 제목에 관련 `FR-xx` (필요 시 `ADR-Qn`)를 명시한다. 예: `feat: RESERVED->CONFIRMED 승인 처리 구현 (FR-09)`
- 사용자가 명시적으로 요청한 경우에만 커밋한다(기존 전역 지침과 동일).

---

## 5. 금지 사항

- **PRD에 없는 기능을 추가하지 않는다.** `docs/PRD.md`의 FR/비범위(Non-goals) 섹션에 없는 동작을 임의로 구현하지 않는다.
- **테스트 없이 커밋하지 않는다.** 새 로직에는 대응하는 테스트를 함께 작성한다(테스트 harness가 아직 없다면 먼저 사용자와 harness 구성을 정한다 — §1 참고).
- **verify 스크립트가 실패한 상태로 커밋하지 않는다.**
- **DECISIONS(`docs/DECISIONS.md`)와 충돌하는 상황을 발견하면, 코드를 임의로 수정하지 말고 사용자에게 먼저 질문한다.** SPEC/DECISIONS에 근거가 없는 회색지대도 동일하게 처리한다(PRD의 OPEN ITEMS 참고).
