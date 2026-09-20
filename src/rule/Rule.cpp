#include "watchflow/rule/Rule.hpp"

using namespace std;

namespace watchflow {

Rule::Rule(
    string name,
    EventType eventType,
    unique_ptr<IMatcher> matcher,
    unique_ptr<IAction> action
) {
    name_ = move(name);
    eventType_ = eventType;
    matcher_ = move(matcher);
    action_ = move(action);
}

bool Rule::matches(const FileEvent& event) const {
    if (event.type != eventType_) {
        return false;
    }

    return matcher_->matches(event);
}

bool Rule::execute(const FileEvent& event) const {
    return action_->execute(event);
}

const string& Rule::name() const {
    return name_;
}

}