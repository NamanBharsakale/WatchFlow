#include "watchflow/action/NotificationAction.hpp"
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

NotificationAction::NotificationAction(std::string message)
    : message_(std::move(message)) {}

bool NotificationAction::execute(const FileEvent& event) {
    const auto message = replaceAll(message_, "{file}", event.path.filename().string());
    const auto command = "notify-send \"WatchFlow\" \"" + message + "\"";
    return std::system(command.c_str()) == 0;
}

} // namespace watchflow
