#include "watchflow/action/FileActionUtils.hpp"

#include <chrono>
#include <sstream>
#include <thread>

namespace watchflow {

std::filesystem::path resolveDuplicatePath(
    const std::filesystem::path& target,
    DuplicateMode mode) {
    if (mode == DuplicateMode::Overwrite || !std::filesystem::exists(target)) {
        return target;
    }

    if (mode == DuplicateMode::Skip) {
        return {};
    }

    const auto parent = target.parent_path();
    const auto stem = target.stem().string();
    const auto extension = target.extension().string();

    for (int counter = 1; counter < 10000; ++counter) {
        std::ostringstream candidateName;
        candidateName << stem << "-" << counter << extension;

        auto candidate = parent / candidateName.str();
        if (!std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return {};
}

bool waitUntilFileStable(
    const std::filesystem::path& file,
    const FileStabilityOptions& options) {
    if (!options.enabled) {
        return true;
    }

    std::error_code ec;
    if (!std::filesystem::exists(file, ec) || ec) {
        return false;
    }

    const auto started = std::chrono::steady_clock::now();
    auto stableSince = std::chrono::steady_clock::now();
    auto previousSize = std::filesystem::file_size(file, ec);
    if (ec) {
        return false;
    }

    while (std::chrono::steady_clock::now() - started < options.timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        const auto currentSize = std::filesystem::file_size(file, ec);
        if (ec) {
            return false;
        }

        if (currentSize != previousSize) {
            previousSize = currentSize;
            stableSince = std::chrono::steady_clock::now();
            continue;
        }

        if (std::chrono::steady_clock::now() - stableSince >= options.stableFor) {
            return true;
        }
    }

    return false;
}

} // namespace watchflow
