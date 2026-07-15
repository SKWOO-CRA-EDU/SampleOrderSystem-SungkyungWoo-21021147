#include "ConsoleApp.h"

#include <iostream>

namespace app {

std::string ConsoleApp::ReadField(const std::string& label) {
    std::cout << label << ": ";
    return input_.ReadLine();
}

void ConsoleApp::Run() {
    while (true) {
        std::cout << "\n=== S-Semi 시료 생산 주문관리 시스템 ===\n"
                     "1.시료등록  2.시료조회  3.시료검색  4.시료삭제\n"
                     "5.시료예약  6.접수된주문목록  7.주문승인  8.주문거절\n"
                     "9.CONFIRMED취소  10.출고처리  11.생산완료처리  12.생산현황/대기주문\n"
                     "13.주문량확인  14.재고량확인  0.종료\n"
                     "명령 선택> ";
        std::string choice = input_.ReadLine();
        if (choice.empty() || choice == "0") {
            std::cout << "종료합니다.\n";
            break;
        }
        if (!DispatchCommand(choice)) {
            std::cout << "알 수 없는 명령입니다: " << choice << "\n";
        }
    }
}

bool ConsoleApp::DispatchCommand(const std::string& choice) {
    try {
        if (choice == "1") {
            std::string sampleId = ReadField("시료ID");
            std::string name = ReadField("이름");
            double avgProductionTime = std::stod(ReadField("평균생산시간"));
            double yield = std::stod(ReadField("수율(0~1)"));
            int yieldNumerator = static_cast<int>(yield * domain::YIELD_DENOMINATOR + 0.5);
            controller_.RegisterSample(sampleId, name, avgProductionTime, yieldNumerator);
        } else if (choice == "2") {
            controller_.ListSamples();
        } else if (choice == "3") {
            controller_.SearchSamples(ReadField("검색어"));
        } else if (choice == "4") {
            controller_.DeleteSample(ReadField("시료ID"));
        } else if (choice == "5") {
            std::string sampleId = ReadField("시료ID");
            std::string customerName = ReadField("고객명");
            int64_t orderQuantity = std::stoll(ReadField("주문수"));
            controller_.ReserveOrder(sampleId, customerName, orderQuantity);
        } else if (choice == "6") {
            controller_.PendingOrderList();
        } else if (choice == "7") {
            controller_.ApproveOrder(std::stoll(ReadField("주문ID")));
        } else if (choice == "8") {
            controller_.RejectOrder(std::stoll(ReadField("주문ID")));
        } else if (choice == "9") {
            controller_.CancelConfirmedOrder(std::stoll(ReadField("주문ID")));
        } else if (choice == "10") {
            controller_.ReleaseOrder(std::stoll(ReadField("주문ID")));
        } else if (choice == "11") {
            controller_.CompleteProduction();
        } else if (choice == "12") {
            controller_.ProductionStatusDisplay();
        } else if (choice == "13") {
            controller_.CheckOrderVolume();
        } else if (choice == "14") {
            controller_.CheckStockLevel();
        } else {
            return false;
        }
    } catch (const std::exception&) {
        std::cout << "입력값을 해석할 수 없습니다. 숫자 형식을 확인하세요.\n";
    }
    return true;
}

}  // namespace app
