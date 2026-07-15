// 콘솔 메뉴 루프. 합성 루트(main.cpp)에 인접한 진입점 드라이버 — CLAUDE.md §2가 iostream 직접
// 호출을 금지하는 "Controller"는 AppController를 가리키며, ConsoleApp은 그 상위의 메뉴/입력 구동
// 계층이다(메뉴 텍스트는 도메인 로직이 아니다). 도메인 결과 표시는 전부 AppController→IOutputPort로 위임한다.
#pragma once

#include "../ports/IInputPort.h"
#include "AppController.h"

namespace app {

class ConsoleApp {
public:
    ConsoleApp(AppController& controller, ports::IInputPort& input) : controller_(controller), input_(input) {}

    void Run();

private:
    AppController& controller_;
    ports::IInputPort& input_;

    std::string ReadField(const std::string& label);

    // CC-11 — 메인 2단 구조. 각 Run*Menu는 [0] 뒤로로 메인 복귀.
    void PrintMainMenu();
    void RunSampleManagementMenu();
    void RunOrderApprovalMenu();
    void RunMonitoringMenu();
    void RunProductionLineMenu();

    // 기존 14개 기능 동작 — 입력 순서/출력 내용은 CC-10 이전과 동일, 도달 경로만 서브메뉴로 이동.
    void DoRegisterSample();
    void DoListSamples();
    void DoSearchSamples();
    void DoDeleteSample();
    void DoReserveOrder();
    void DoPendingOrderList();
    void DoApproveOrder();
    void DoRejectOrder();
    void DoCancelConfirmedOrder();
    void DoReleaseOrder();
    void DoCompleteProduction();
    void DoProductionStatusDisplay();
    void DoCheckOrderVolume();
    void DoCheckStockLevel();
};

}  // namespace app
