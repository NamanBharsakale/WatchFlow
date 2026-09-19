#pragma once

#include "watchflow/core/FileEvent.hpp"
#include "watchflow/rule/Rule.hpp"
#include <string>
#include <utility>
#include <vector>

namespace watchflow {

class RuleEngine {
public:
    void addRule(Rule rule);
    std::vector<std::pair<std::string, bool>> onEvent(const FileEvent& event) const;

private:
    std::vector<Rule> rules_;
};

} // namespace watchflow
