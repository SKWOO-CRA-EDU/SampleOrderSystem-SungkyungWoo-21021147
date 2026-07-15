#include "Harness.h"

#include <iostream>
#include <vector>

namespace harness {

namespace {
int passCount = 0;
int failCount = 0;
std::vector<std::string> failures;
}  // namespace

void Check(bool condition, const std::string& description) {
    if (condition) {
        ++passCount;
    } else {
        ++failCount;
        failures.push_back(description);
        std::cout << "[FAIL] " << description << "\n";
    }
}

int Report() {
    std::cout << "\n==== 결과: " << passCount << " passed, " << failCount << " failed"
              << " (총 " << (passCount + failCount) << "건) ====\n";
    if (!failures.empty()) {
        std::cout << "실패 목록:\n";
        for (const auto& f : failures) std::cout << "  - " << f << "\n";
    }
    return failCount == 0 ? 0 : 1;
}

}  // namespace harness
