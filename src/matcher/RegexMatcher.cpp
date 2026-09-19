#include "watchflow/matcher/RegexMatcher.hpp"

namespace watchflow {

RegexMatcher::RegexMatcher(std::string pattern)
    : pattern_(std::move(pattern)) {}

bool RegexMatcher::matches(const FileEvent& event) const {
    return std::regex_match(event.path.filename().string(), pattern_);
}

} // namespace watchflow
