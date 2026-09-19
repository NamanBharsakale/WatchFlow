#pragma once

#include "watchflow/core/FileEvent.hpp"
#include "watchflow/rule/Rule.hpp"
#include <vector>

namespace watchflow {

class RuleEngine {
public:
    void addRule(Rule rule);
    void onEvent(const FileEvent& event) const;

private:
    std::vector<Rule> rules_;
};

} // namespace watchflow
