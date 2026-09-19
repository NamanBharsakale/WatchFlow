#pragma once

#include "watchflow/matcher/IMatcher.hpp"
#include <set>
#include <string>
#include <vector>

namespace watchflow {

class ExtensionMatcher final : public IMatcher {
public:
    explicit ExtensionMatcher(std::vector<std::string> extensions);
    bool matches(const FileEvent& event) const override;

private:
    std::set<std::string> extensions_;
};

} // namespace watchflow
