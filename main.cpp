// 합성 루트. 저장소/포트 구현체를 조립해 Model/Controller/View에 주입한다 (CLAUDE.md §2).
#include <iostream>

#include <Windows.h>

#include "src/app/AppController.h"
#include "src/app/ConsoleApp.h"
#include "src/infra/console/ConsoleInputPort.h"
#include "src/infra/console/ConsoleOutputPort.h"
#include "src/infra/console/SystemClock.h"
#include "src/infra/persistence/FileOrderRepository.h"
#include "src/infra/persistence/FileSampleRepository.h"
#include "src/infra/persistence/JsonDocumentStore.h"
#include "src/model/OrderService.h"
#include "src/model/SampleService.h"

int main() {
    // 콘솔 코드페이지를 UTF-8로 설정 (ADR-Q25) — /utf-8 컴파일 옵션과 콘솔 기본 코드페이지 불일치 해소.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // CC-11 — ANSI 색상 표시를 위해 가상 터미널 처리 활성화. 실패해도 프로그램은 계속 진행한다.
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode = 0;
    if (hStdOut != INVALID_HANDLE_VALUE && GetConsoleMode(hStdOut, &consoleMode)) {
        SetConsoleMode(hStdOut, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    infra::persistence::JsonDocumentStore store("data.json");
    infra::persistence::FileSampleRepository sampleRepo(store);
    infra::persistence::FileOrderRepository orderRepo(store);
    infra::console::SystemClock clock;

    model::SampleService sampleService(sampleRepo, orderRepo);
    model::OrderService orderService(sampleRepo, orderRepo, clock);

    infra::console::ConsoleOutputPort output;
    infra::console::ConsoleInputPort input;
    app::AppController controller(sampleService, orderService, output);
    app::ConsoleApp consoleApp(controller, input);

    try {
        consoleApp.Run();
    } catch (const std::exception& e) {
        // [F] 치명 범주 — 앱 경계에서 1회 포착 후 종료 [ADR-E1]
        std::cerr << "치명적 오류로 종료합니다: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
