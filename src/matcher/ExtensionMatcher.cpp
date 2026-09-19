#include "watchflow/matcher/ExtensionMatcher.hpp"

#include <algorithm>
#include <cctype>

namespace watchflow {

namespace {
std::string normalizeExtension(std::string extension) {
    if (!extension.empty() && extension.front() == '.') {
        extension.erase(extension.begin());
    }

    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char value) {
                       return static_cast<char>(std::tolower(value));
                   });

    return extension;
}
}

ExtensionMatcher::ExtensionMatcher(std::vector<std::string> extensions) {
    for (auto& extension : extensions) {
        extensions_.insert(normalizeExtension(std::move(extension)));
    }
}

bool ExtensionMatcher::matches(const FileEvent& event) const {
    auto extension = event.path.extension().string();
    return extensions_.contains(normalizeExtension(std::move(extension)));
}

} // namespace watchflow
