#pragma once

#include "watchflow/action/IAction.hpp"
#include <filesystem>

namespace watchflow {

class CopyAction final : public IAction {
public:
    explicit CopyAction(std::filesystem::path destination);
    bool execute(const FileEvent& event) override;

private:
    std::filesystem::path destination_;
};

} // namespace watchflow
