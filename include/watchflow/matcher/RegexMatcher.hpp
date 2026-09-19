#pragma once

#include "watchflow/matcher/IMatcher.hpp"
#include <regex>
#include <string>

namespace watchflow {

class RegexMatcher final : public IMatcher {
public:
    explicit RegexMatcher(std::string pattern);
    bool matches(const FileEvent& event) const override;

private:
    std::regex pattern_;
};

} // namespace watchflow
