#include "watchflow/action/CopyAction.hpp"
#include <filesystem>

namespace watchflow {

CopyAction::CopyAction(std::filesystem::path destination)
    : destination_(std::move(destination)) {}

bool CopyAction::execute(const FileEvent& event) {
    if (!std::filesystem::exists(event.path)) return false;

    std::error_code ec;
    std::filesystem::create_directories(destination_, ec);
    if (ec) return false;

    std::filesystem::copy_file(
        event.path,
        destination_ / event.path.filename(),
        std::filesystem::copy_options::overwrite_existing,
        ec
    );
    return !ec;
}

} // namespace watchflow
