#include "ConsoleApp.h"

#include <iostream>

namespace app {

namespace {
// CC-11 — 메뉴 번호 강조 전용 색상. 색이 꺼져도 "[N]" 텍스트 자체로 의미가 통한다.
constexpr const char* kColorReset = "\x1b[0m";
constexpr const char* kColorMenuNumber = "\x1b[1;36m";

std::string Num(const std::string& n) { return std::string(kColorMenuNumber) + "[" + n + "]" + kColorReset; }
}  // namespace

std::string ConsoleApp::ReadField(const std::string& label) {
    std::cout << label << ": ";
    return input_.ReadLine();
}

void ConsoleApp::PrintMainMenu() {
    auto summary = controller_.GetSampleSummary();
    std::cout << "\n=== S-Semi 시료 생산 주문관리 시스템 ===\n"
                 "등록 시료 종수: "
              << summary.sampleCount << "종  /  총 재고수량: " << summary.totalStockQuantity
              << "개\n"
                 "----------------------------------------\n"
              << Num("1") << " 시료 관리\n"
              << Num("2") << " 시료 주문\n"
              << Num("3") << " 주문 승인/거절\n"
              << Num("4") << " 모니터링\n"
              << Num("5") << " 생산라인 조회\n"
              << Num("6") << " 출고 처리\n"
              << Num("0") << " 종료\n"
                 "명령 선택> ";
}

void ConsoleApp::Run() {
    while (true) {
        PrintMainMenu();
        std::string choice = input_.ReadLine();
        if (choice.empty() || choice == "0") {
            std::cout << "종료합니다.\n";
            break;
        }
        if (choice == "1") {
            RunSampleManagementMenu();
        } else if (choice == "2") {
            DoReserveOrder();
        } else if (choice == "3") {
            RunOrderApprovalMenu();
        } else if (choice == "4") {
            RunMonitoringMenu();
        } else if (choice == "5") {
            RunProductionLineMenu();
        } else if (choice == "6") {
            DoReleaseOrder();
        } else {
            std::cout << "알 수 없는 명령입니다: " << choice << "\n";
        }
    }
}

void ConsoleApp::RunSampleManagementMenu() {
    while (true) {
        std::cout << "\n-- 시료 관리 --\n"
                  << Num("1") << " 시료등록\n"
                  << Num("2") << " 시료조회\n"
                  << Num("3") << " 시료검색\n"
                  << Num("4") << " 시료삭제\n"
                  << Num("0") << " 뒤로\n"
                     "명령 선택> ";
        std::string choice = input_.ReadLine();
        if (choice.empty() || choice == "0") return;
        if (choice == "1") {
            DoRegisterSample();
        } else if (choice == "2") {
            DoListSamples();
        } else if (choice == "3") {
            DoSearchSamples();
        } else if (choice == "4") {
            DoDeleteSample();
        } else {
            std::cout << "알 수 없는 명령입니다: " << choice << "\n";
        }
    }
}

void ConsoleApp::RunOrderApprovalMenu() {
    while (true) {
        std::cout << "\n-- 주문 승인/거절 --\n"
                  << Num("1") << " 접수된주문목록\n"
                  << Num("2") << " 주문승인\n"
                  << Num("3") << " 주문거절\n"
                  << Num("4") << " CONFIRMED취소\n"
                  << Num("0") << " 뒤로\n"
                     "명령 선택> ";
        std::string choice = input_.ReadLine();
        if (choice.empty() || choice == "0") return;
        if (choice == "1") {
            DoPendingOrderList();
        } else if (choice == "2") {
            DoApproveOrder();
        } else if (choice == "3") {
            DoRejectOrder();
        } else if (choice == "4") {
            DoCancelConfirmedOrder();
        } else {
            std::cout << "알 수 없는 명령입니다: " << choice << "\n";
        }
    }
}

void ConsoleApp::RunMonitoringMenu() {
    while (true) {
        std::cout << "\n-- 모니터링 --\n"
                  << Num("1") << " 주문량확인\n"
                  << Num("2") << " 재고량확인\n"
                  << Num("0") << " 뒤로\n"
                     "명령 선택> ";
        std::string choice = input_.ReadLine();
        if (choice.empty() || choice == "0") return;
        if (choice == "1") {
            DoCheckOrderVolume();
        } else if (choice == "2") {
            DoCheckStockLevel();
        } else {
            std::cout << "알 수 없는 명령입니다: " << choice << "\n";
        }
    }
}

void ConsoleApp::RunProductionLineMenu() {
    while (true) {
        std::cout << "\n-- 생산라인 조회 --\n"
                  << Num("1") << " 생산현황/대기주문\n"
                  << Num("2") << " 생산완료처리\n"
                  << Num("0") << " 뒤로\n"
                     "명령 선택> ";
        std::string choice = input_.ReadLine();
        if (choice.empty() || choice == "0") return;
        if (choice == "1") {
            DoProductionStatusDisplay();
        } else if (choice == "2") {
            DoCompleteProduction();
        } else {
            std::cout << "알 수 없는 명령입니다: " << choice << "\n";
        }
    }
}

void ConsoleApp::DoRegisterSample() {
    try {
        std::string sampleId = ReadField("시료ID");
        std::string name = ReadField("이름");
        double avgProductionTime = std::stod(ReadField("평균생산시간"));
        double yield = std::stod(ReadField("수율(0~1)"));
        int yieldNumerator = static_cast<int>(yield * domain::YIELD_DENOMINATOR + 0.5);
        controller_.RegisterSample(sampleId, name, avgProductionTime, yieldNumerator);
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoListSamples() {
    try {
        controller_.ListSamples();
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoSearchSamples() {
    try {
        controller_.SearchSamples(ReadField("검색어"));
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoDeleteSample() {
    try {
        controller_.DeleteSample(ReadField("시료ID"));
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoReserveOrder() {
    try {
        std::string sampleId = ReadField("시료ID");
        std::string customerName = ReadField("고객명");
        int64_t orderQuantity = std::stoll(ReadField("주문수"));
        controller_.ReserveOrder(sampleId, customerName, orderQuantity);
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoPendingOrderList() {
    try {
        controller_.PendingOrderList();
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoApproveOrder() {
    try {
        controller_.ApproveOrder(std::stoll(ReadField("주문ID")));
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoRejectOrder() {
    try {
        controller_.RejectOrder(std::stoll(ReadField("주문ID")));
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoCancelConfirmedOrder() {
    try {
        controller_.CancelConfirmedOrder(std::stoll(ReadField("주문ID")));
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoReleaseOrder() {
    try {
        controller_.ReleaseOrder(std::stoll(ReadField("주문ID")));
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoCompleteProduction() {
    try {
        controller_.CompleteProduction();
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoProductionStatusDisplay() {
    try {
        controller_.ProductionStatusDisplay();
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoCheckOrderVolume() {
    try {
        controller_.CheckOrderVolume();
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

void ConsoleApp::DoCheckStockLevel() {
    try {
        controller_.CheckStockLevel();
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
}

}  // namespace app
