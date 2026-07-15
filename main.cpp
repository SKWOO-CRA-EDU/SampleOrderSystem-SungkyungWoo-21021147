// 합성 루트. 저장소/포트 구현체를 조립해 Model/Controller/View에 주입한다 (CLAUDE.md §2).
#include <iostream>

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
