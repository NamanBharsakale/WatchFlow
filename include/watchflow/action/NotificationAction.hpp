#pragma once

#include "watchflow/action/IAction.hpp"
#include <string>

namespace watchflow {

class NotificationAction final : public IAction {
public:
    explicit NotificationAction(std::string message);
    bool execute(const FileEvent& event) override;

private:
    std::string message_;
};

} // namespace watchflow
