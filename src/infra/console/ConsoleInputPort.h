// IInputPort의 실제(Real) 구현체. std::cin으로 한 줄씩 읽는다.
#pragma once

#include "../../ports/IInputPort.h"

namespace infra::console {

class ConsoleInputPort : public ports::IInputPort {
public:
    std::string ReadLine() override;
};

}  // namespace infra::console
