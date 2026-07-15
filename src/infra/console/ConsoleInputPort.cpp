#include "ConsoleInputPort.h"

#include <iostream>

namespace infra::console {

std::string ConsoleInputPort::ReadLine() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        return "";
    }
    return line;
}

}  // namespace infra::console
