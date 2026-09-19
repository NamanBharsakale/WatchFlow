#include "watchflow/action/ExecuteAction.hpp"
#include <cstdlib>

namespace watchflow {

namespace {
std::string replaceAll(std::string text, const std::string& from, const std::string& to) {
    std::size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
    return text;
}
}

ExecuteAction::ExecuteAction(std::string command)
    : command_(std::move(command)) {}

bool ExecuteAction::execute(const FileEvent& event) {
    const auto command = replaceAll(command_, "{file}", event.path.string());
    return std::system(command.c_str()) == 0;
}

} // namespace watchflow
