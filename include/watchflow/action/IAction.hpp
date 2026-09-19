#pragma once

#include "watchflow/core/FileEvent.hpp"

namespace watchflow {

class IAction {
public:
    virtual ~IAction() = default;
    virtual bool execute(const FileEvent& event) = 0;
};

} // namespace watchflow
