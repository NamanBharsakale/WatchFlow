#include "watchflow/rule/Rule.hpp"

namespace watchflow {

Rule::Rule(std::string name,
           EventType eventType,
           std::unique_ptr<IMatcher> matcher,
           std::unique_ptr<IAction> action)
    : name_(std::move(name)),
      eventType_(eventType),
      matcher_(std::move(matcher)),
      action_(std::move(action)) {}

bool Rule::matches(const FileEvent& event) const {
    return event.type == eventType_ && matcher_->matches(event);
}

bool Rule::execute(const FileEvent& event) const {
    return action_->execute(event);
}

const std::string& Rule::name() const {
    return name_;
}

} // namespace watchflow
