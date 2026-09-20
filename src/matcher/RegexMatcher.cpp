#include "watchflow/matcher/RegexMatcher.hpp"

using namespace std;
using namespace watchflow;

RegexMatcher::RegexMatcher(string pattern)
{
    pattern_ = pattern;
}

bool RegexMatcher::matches(const FileEvent& event) const
{
    string filename = event.path.filename().string();

    return regex_match(filename, pattern_);
}