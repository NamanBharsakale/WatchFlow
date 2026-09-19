#include "watchflow/rule/RuleEngine.hpp"
#include <iostream>

namespace watchflow {

void RuleEngine::addRule(Rule rule) {
    rules_.push_back(std::move(rule));
}

std::vector<std::pair<std::string, bool>> RuleEngine::onEvent(const FileEvent& event) const {
    std::vector<std::pair<std::string, bool>> results;

    for (const auto& rule : rules_) {
        if (!rule.matches(event)) continue;

        std::cout << "[RULE] " << rule.name() << "\n";
        const bool success = rule.execute(event);
        std::cout << (success ? "[SUCCESS]\n" : "[FAILED]\n");
        results.emplace_back(rule.name(), success);
    }

    return results;
}

} // namespace watchflow
