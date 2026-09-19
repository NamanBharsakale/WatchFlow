#pragma once

#include "watchflow/action/IAction.hpp"
#include <string>

namespace watchflow {

class ExecuteAction final : public IAction {
public:
    explicit ExecuteAction(std::string command);
    bool execute(const FileEvent& event) override;

private:
    std::string command_;
};

} // namespace watchflow
