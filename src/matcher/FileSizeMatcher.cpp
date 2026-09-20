#include "watchflow/matcher/FileSizeMatcher.hpp"

#include <filesystem>

using namespace std;
using namespace watchflow;

FileSizeMatcher::FileSizeMatcher(uintmax_t bytes, SizeComparison comparison)
{
    bytes_ = bytes;
    comparison_ = comparison;
}

bool FileSizeMatcher::matches(const FileEvent& event) const
{
    error_code ec;

    uintmax_t size = filesystem::file_size(event.path, ec);

    // If file size cannot be read
    if (ec)
        return false;

    switch (comparison_)
    {
        case SizeComparison::GreaterThan:
            return size > bytes_;

        case SizeComparison::LessThan:
            return size < bytes_;

        case SizeComparison::Equal:
            return size == bytes_;
    }

    return false;
}