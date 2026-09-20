#include "watchflow/matcher/ExtensionMatcher.hpp"

#include <algorithm>
#include <cctype>

using namespace std;
using namespace watchflow;

string normalizeExtension(string extension)
{
    // Remove '.' from beginning
    if (!extension.empty() && extension[0] == '.')
        extension.erase(0, 1);

    // Convert to lowercase
    transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) {
            return tolower(c);
        });

    return extension;
}

ExtensionMatcher::ExtensionMatcher(vector<string> extensions)
{
    for (string extension : extensions)
    {
        extensions_.insert(normalizeExtension(extension));
    }
}

bool ExtensionMatcher::matches(const FileEvent& event) const
{
    string extension = event.path.extension().string();

    return extensions_.contains(
        normalizeExtension(extension)
    );
}