#include "watchflow/action/MoveAction.hpp"

#include <filesystem>

namespace watchflow {

MoveAction::MoveAction(std::filesystem::path destination,
                       ActionOptions options)
    : destination_(std::move(destination)),
      options_(options) {
}

bool MoveAction::execute(const FileEvent& event) {

    // Check if file exists
    if (!std::filesystem::exists(event.path)) {
        return false;
    }

    // Create destination directory
    std::filesystem::create_directories(destination_);

    // Create target path
    auto target = destination_ / event.path.filename();

    // Move the file
    std::error_code error;

    std::filesystem::rename(
        event.path,
        target,
        error
    );

    // Return whether move succeeded
    return !error;
}

}
