
#include "watchflow/action/CopyAction.hpp"
#include "watchflow/action/FileActionUtils.hpp"

#include <filesystem>

using namespace watchflow;

using std::error_code;
using std::filesystem::copy_file;
using std::filesystem::copy_options;
using std::filesystem::create_directories;
using std::filesystem::exists;
using std::filesystem::path;
using std::move;

CopyAction::CopyAction(path destination, ActionOptions options)
    : destination_(move(destination)),
      options_(options) {
}

bool CopyAction::execute(const FileEvent& event) {

    // Check if file exists
    if (!exists(event.path))
        return false;

    // Wait until file is completely written
    if (!waitUntilFileStable(event.path, options_.fileStability))
        return false;

    // Create destination directory
    error_code ec;
    create_directories(destination_, ec);

    if (ec)
        return false;

    // Find final destination path
    auto target = resolveDuplicatePath(
        destination_ / event.path.filename(),
        options_.duplicateMode
    );

    if (target.empty())
        return false;

    // Copy the file
    copy_file(
        event.path,
        target,
        copy_options::overwrite_existing,
        ec
    );

    return !ec;
}
