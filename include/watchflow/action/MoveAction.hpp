#pragma once

#include "watchflow/action/IAction.hpp"
#include "watchflow/config/ConfigManager.hpp"
#include <filesystem>

namespace watchflow {

class MoveAction final : public IAction {
public:
    explicit MoveAction(std::filesystem::path destination,
                        ActionOptions options = {});
    bool execute(const FileEvent& event) override;

private:
    std::filesystem::path destination_;
    ActionOptions options_;
};

} // namespace watchflow
