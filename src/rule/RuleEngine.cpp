#include "watchflow/rule/RuleEngine.hpp"
#include <iostream>

namespace watchflow {

void RuleEngine::addRule(Rule rule) {
    rules_.push_back(std::move(rule));
}

void RuleEngine::onEvent(const FileEvent& event) const {
    for (const auto& rule : rules_) {
        if (!rule.matches(event)) continue;

        std::cout << "[RULE] " << rule.name() << "\n";
        const bool success = rule.execute(event);
        std::cout << (success ? "[SUCCESS]\n" : "[FAILED]\n");
    }
}

} // namespace watchflow
