#include "watchflow/action/CompressAction.hpp"
#include <cstdlib>
#include <filesystem>
#include <string>

namespace watchflow {

namespace {
std::string shellQuote(const std::string& value) {
    std::string result = "'";
    for (char c : value) {
        if (c == '\'') result += "'\\''";
        else result += c;
    }
    result += "'";
    return result;
}
}

CompressAction::CompressAction(std::string outputSuffix)
    : outputSuffix_(std::move(outputSuffix)) {}

bool CompressAction::execute(const FileEvent& event) {
    if (!std::filesystem::exists(event.path)) return false;

    const auto output = event.path.string() + outputSuffix_;
    const auto command =
        "gzip -c " + shellQuote(event.path.string()) +
        " > " + shellQuote(output);

    return std::system(command.c_str()) == 0;
}

} // namespace watchflow
