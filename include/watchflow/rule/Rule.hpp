#pragma once

#include "watchflow/action/IAction.hpp"
#include "watchflow/matcher/IMatcher.hpp"
#include "watchflow/core/FileEvent.hpp"
#include <memory>
#include <string>

namespace watchflow {

class Rule {
public:
    Rule(std::string name,
         EventType eventType,
         std::unique_ptr<IMatcher> matcher,
         std::unique_ptr<IAction> action);

    bool matches(const FileEvent& event) const;
    bool execute(const FileEvent& event) const;
    const std::string& name() const;

private:
    std::string name_;
    EventType eventType_;
    std::unique_ptr<IMatcher> matcher_;
    std::unique_ptr<IAction> action_;
};

} // namespace watchflow
