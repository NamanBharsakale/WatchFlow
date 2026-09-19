#include "watchflow/action/MoveAction.hpp"
#include "watchflow/action/FileActionUtils.hpp"

#include <filesystem>

namespace watchflow {

MoveAction::MoveAction(std::filesystem::path destination,
                       ActionOptions options)
    : destination_(std::move(destination)),
      options_(options) {}

bool MoveAction::execute(const FileEvent& event) {
    if (!std::filesystem::exists(event.path)) return false;
    if (!waitUntilFileStable(event.path, options_.fileStability)) return false;

    std::error_code ec;
    std::filesystem::create_directories(destination_, ec);
    if (ec) return false;

    const auto target = resolveDuplicatePath(
        destination_ / event.path.filename(),
        options_.duplicateMode);

    if (target.empty()) return false;

    std::filesystem::rename(event.path, target, ec);
    if (!ec) return true;
    if (std::filesystem::exists(target)) return true;

    ec.clear();
    std::filesystem::copy_file(
        event.path,
        target,
        std::filesystem::copy_options::overwrite_existing,
        ec);
    if (ec) return false;

    std::filesystem::remove(event.path, ec);
    return !ec;
}

} // namespace watchflow
