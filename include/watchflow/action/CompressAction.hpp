#pragma once

#include "watchflow/action/IAction.hpp"
#include <string>

namespace watchflow {

class CompressAction final : public IAction {
public:
    explicit CompressAction(std::string outputSuffix = ".gz");
    bool execute(const FileEvent& event) override;

private:
    std::string outputSuffix_;
};

} // namespace watchflow
