#pragma once

#include "watchflow/config/ConfigManager.hpp"
#include <filesystem>

namespace watchflow {

std::filesystem::path resolveDuplicatePath(
    const std::filesystem::path& target,
    DuplicateMode mode);

bool waitUntilFileStable(
    const std::filesystem::path& file,
    const FileStabilityOptions& options);

} // namespace watchflow
