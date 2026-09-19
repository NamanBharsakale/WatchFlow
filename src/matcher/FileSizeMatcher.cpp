#include "watchflow/matcher/FileSizeMatcher.hpp"
#include <filesystem>

namespace watchflow {

FileSizeMatcher::FileSizeMatcher(std::uintmax_t bytes, SizeComparison comparison)
    : bytes_(bytes), comparison_(comparison) {}

bool FileSizeMatcher::matches(const FileEvent& event) const {
    std::error_code ec;
    const auto size = std::filesystem::file_size(event.path, ec);
    if (ec) return false;

    switch (comparison_) {
        case SizeComparison::GreaterThan: return size > bytes_;
        case SizeComparison::LessThan:    return size < bytes_;
        case SizeComparison::Equal:       return size == bytes_;
    }
    return false;
}

} // namespace watchflow
