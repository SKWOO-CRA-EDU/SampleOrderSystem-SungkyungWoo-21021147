# tests — Harness (ADR-I4: assert-count, 프레임워크 없음)

## 실행

```
msbuild tests\SampleOrderSystemTests.vcxproj /p:Configuration=Debug /p:Platform=x64
tests\x64\Debug\SampleOrderSystemTests.exe
```

프레임워크 의존성 0(ADR-I4). `Harness.h`의 `harness::Check(condition, description)`이 통과/실패를
집계하고, `harness::Report()`가 요약을 출력하며 실패가 있으면 exit code 1을 반환한다.

## 구성

| 파일 | 관측 축(ADR-H1~H4) | 다루는 명제 |
|---|---|---|
| `CalculationTests.cpp` | Model 반환값(순수 함수, `src/model/Calculations.h`) | #1, #2, #3, #4, #5 |
| `StockTransitionTests.cpp` | Repository 스냅샷(`FindById`) + Model 반환값 | #6, #7, #9, #10, #11, #12 |
| `RepositoryInvariantTests.cpp` | Repository 스냅샷 + Model 반환값 | #17, #18, #24, #25 |
| `PersistenceFailureTests.cpp` | Tier2(실제 파일 실패 주입, ADR-H4) | #29 |
| `VerificationScenarioTests.cpp` | Repository 스냅샷 + Model 반환값 | DECISIONS.md ADR-Q21~Q24 ★ 검산 시나리오 |

`fakes/FakeClock.h`는 ADR-H3(합성 루트가 Clock 구현체를 주입)에 따른 결정적 시각 테스트 더블이다 —
실제 `sleep`은 쓰지 않는다.

## 이번 회차가 덮은 명제 (docs/HARNESS-SURVEY.md 축1, 총 33개 중)

`#1 #2 #3 #4 #5 #6 #7 #9 #10 #11 #12 #17 #18 #24 #25 #29` — 16개.

플러스 `docs/DECISIONS.md` §1 "★ 검산" 시나리오(재고 30, order1=100, order2=20, 수율 0.92,
order1 T2 → order2 T2 → order1 T4 → order2 T4 → 둘 다 출고, 최종 재고 0)를 통합 시나리오로 별도 검증.

## 이번 회차가 덮지 않은 명제 (명시적으로 범위 제외 — 지시에 따름)

`#8 #13 #14 #15 #16 #19 #20 #21 #22 #23 #26 #27 #28 #30 #31 #32 #33` — 17개.

- **#8**(T4 재고증가)은 `VerificationScenarioTests.cpp`에서 실질적으로 함께 검증된다(order1/order2 T4
  전후 재고 diff를 직접 단언) — 별도 propositio 번호로 분리 표기하지 않았을 뿐 커버리지 공백은 아니다.
- 나머지는 이번 세션의 지시 범위(`#1~4, #5/#8, #6/#7/#9, #10~12, #17/#18, #24/#25, #29`) 밖이라
  의도적으로 다루지 않았다. 특히:
  - **#13/#14/#15**(등록 시점 검증 강제 자체) — `SampleService::RegisterSample`이 구현은 하고 있으나
    (`InvalidYield`/`InvalidAvgProductionTime` 거부) 전용 테스트가 없다.
  - **#16**(단일 생산라인 불변식), **#19~#23**(전이 적법성/미지원 전이 거부/REJECTED·RELEASE 종단) —
    `OrderService`의 `RejectOrder`/`CancelConfirmedOrder`/`ApproveOrder`가 상태 가드를 구현하고
    있으나(R5) 전용 테스트가 없다.
  - **#26/#27/#28** — `ADR-E10`(불변식 위반 시 `StorageCorrupted`)은 #29 테스트가 부분적으로 겹쳐
    다루지만 #26 자체(24/25 위반→손상 처리)와 #27(Repository 검증 범위 한정), #28(Sample Delete
    참조무결성 가드)은 전용 테스트가 없다. `SampleService::DeleteSample`은 구현되어 있다
    (`RepositoryInvariantTests.cpp`가 간접적으로 사용하지 않음).
  - **#30/#31**은 #29와 같은 Tier2 계열이나(스키마 버전/손상 레코드) 이번엔 schemaVersion
    불일치만 다루고 "손상 레코드 1건→전체 손상"(#31)의 전용 케이스는 별도로 안 만들었다 —
    실제로는 `PersistenceFailureTests.cpp`의 손상 JSON 테스트가 #31과 사실상 같은 경로를
    통과하지만, 공식적으로 #31 번호로 명시하지는 않았다.
  - **#32**(CreatedAt 불변) — `ADR-H3`(FakeClock)까지는 구성했으나 전용 단언은 없다.
  - **#33**(`IOrderRepository`에 Delete/NextOrderId 없음, 정적 선언 검사·ADR-H6) — 컴파일이
    성공한다는 사실 자체가 `IOrderRepository.h`에 그런 메서드가 없음을 이미 증명하지만
    (있었다면 `InMemoryOrderRepository`/`FileOrderRepository`가 이를 override하지 않는 한
    추상 클래스 인스턴스화 시도로 컴파일이 실패했을 것), grep/static_assert 형태의 명시적
    검사 코드는 작성하지 않았다.

## 커버하지 않은 Harness 관측 축

- **ADR-H5(View 관통 관측, `RecordingOutputPort`)** — 이번 회차에서 다루는 명제 목록에 Controller→View
  호출을 관측해야 하는 항목이 없어 `RecordingOutputPort`/`FakeInputPort` 테스트 더블을 만들지 않았다.
- **ADR-H6(정적 선언 검사, `[[nodiscard]]` grep/static_assert)** — #33 관련 설명대로 전용 검사
  스크립트는 없다. 대신 `src/model/OrderService.cpp`의 보상 롤백 호출에 `(void)` 캐스트가 필요했다는
  사실 자체가, `[[nodiscard]]`가 실제로 강제되고 있음을 빌드 시점에 이미 증명했다(C4834 경고 발생 → 수정).
- **ADR-R2(상태전이·재고증감 원자성) 전용 저장 실패 롤백 테스트** — `OrderService::ApplyOrderTransition`은
  구현되어 있으나(보상 롤백), Repository 테스트 더블에 임의 실패를 주입해 "재고수량이 전이 이전 값으로
  유지되는지" 직접 검증하는 Tier1 테스트는 이번 범위에 없었다(지시된 propositio 목록 밖).

향후 회차에서 이 문서를 갱신할 때는 "이번 회차가 덮은 명제" 표를 새로 추가하고, 위 "덮지 않은 명제"
목록에서 옮겨 적는 방식으로 누적 관리한다.
