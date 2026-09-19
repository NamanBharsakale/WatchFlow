#pragma once

#include "watchflow/matcher/IMatcher.hpp"
#include <cstdint>

namespace watchflow {

enum class SizeComparison {
    GreaterThan,
    LessThan,
    Equal
};

class FileSizeMatcher final : public IMatcher {
public:
    FileSizeMatcher(std::uintmax_t bytes, SizeComparison comparison);
    bool matches(const FileEvent& event) const override;

private:
    std::uintmax_t bytes_;
    SizeComparison comparison_;
};

} // namespace watchflow
