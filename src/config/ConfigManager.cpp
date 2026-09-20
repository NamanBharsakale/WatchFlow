#include "watchflow/config/ConfigManager.hpp"

#include "watchflow/action/CopyAction.hpp"
#include "watchflow/action/MoveAction.hpp"
#include "watchflow/action/ExecuteAction.hpp"
#include "watchflow/action/CompressAction.hpp"
#include "watchflow/action/NotificationAction.hpp"

#include "watchflow/matcher/RegexMatcher.hpp"
#include "watchflow/matcher/ExtensionMatcher.hpp"
#include "watchflow/matcher/FileSizeMatcher.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

using namespace std;

namespace watchflow {


// Remove spaces from beginning and end
string trim(string text)
{
    size_t start = text.find_first_not_of(" \t");

    if (start == string::npos)
        return "";

    size_t end = text.find_last_not_of(" \t");

    return text.substr(start, end - start + 1);
}


// Convert string to uppercase
string upper(string text)
{
    transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) {
            return toupper(c);
        });

    return text;
}


// Split string using delimiter
vector<string> split(string text, char delimiter)
{
    vector<string> result;
    string item;

    stringstream ss(text);

    while (getline(ss, item, delimiter))
    {
        result.push_back(trim(item));
    }

    return result;
}


// Convert event string to EventType
EventType parseEvent(string value)
{
    value = upper(trim(value));

    if (value == "CREATED")
        return EventType::Created;

    if (value == "MODIFIED")
        return EventType::Modified;

    if (value == "DELETED")
        return EventType::Deleted;

    if (value == "MOVED")
        return EventType::Moved;

    throw runtime_error("Unknown event: " + value);
}


// Convert duplicate string to DuplicateMode
DuplicateMode parseDuplicateMode(string value)
{
    value = upper(trim(value));

    if (value == "OVERWRITE")
        return DuplicateMode::Overwrite;

    if (value == "RENAME")
        return DuplicateMode::Rename;

    if (value == "SKIP")
        return DuplicateMode::Skip;

    throw runtime_error("Unknown duplicate mode: " + value);
}


// Create matcher
unique_ptr<IMatcher> createMatcher(
    string type,
    string value)
{
    type = upper(trim(type));

    if (type == "REGEX")
    {
        return make_unique<RegexMatcher>(value);
    }

    if (type == "EXT")
    {
        vector<string> extensions = split(value, ',');

        return make_unique<ExtensionMatcher>(extensions);
    }

    if (type == "SIZE_GT")
    {
        return make_unique<FileSizeMatcher>(
            stoull(value),
            SizeComparison::GreaterThan
        );
    }

    if (type == "SIZE_LT")
    {
        return make_unique<FileSizeMatcher>(
            stoull(value),
            SizeComparison::LessThan
        );
    }

    if (type == "SIZE_EQ")
    {
        return make_unique<FileSizeMatcher>(
            stoull(value),
            SizeComparison::Equal
        );
    }

    throw runtime_error("Unknown matcher: " + type);
}


// Create action
unique_ptr<IAction> createAction(
    string type,
    string value,
    ActionOptions options)
{
    type = upper(trim(type));

    if (type == "COPY")
    {
        return make_unique<CopyAction>(
            value,
            options
        );
    }

    if (type == "MOVE")
    {
        return make_unique<MoveAction>(
            value,
            options
        );
    }

    if (type == "EXECUTE")
    {
        return make_unique<ExecuteAction>(value);
    }

    if (type == "COMPRESS")
    {
        if (value.empty())
            value = ".gz";

        return make_unique<CompressAction>(value);
    }

    if (type == "NOTIFY")
    {
        return make_unique<NotificationAction>(value);
    }

    throw runtime_error("Unknown action: " + type);
}


// Load pipe-based configuration
Config loadConfig(ifstream& input)
{
    Config config;

    string line;

    while (getline(input, line))
    {
        line = trim(line);

        // Ignore empty lines
        if (line.empty())
            continue;

        // Ignore comments
        if (line[0] == '#')
            continue;


        vector<string> parts = split(line, '|');


        // WATCH|directory
        if (parts[0] == "WATCH")
        {
            config.watchDirectory = parts[1];
        }


        // LOG|file
        else if (parts[0] == "LOG")
        {
            config.logFile = parts[1];
        }


        // DUPLICATES|rename
        else if (parts[0] == "DUPLICATES")
        {
            config.actionOptions.duplicateMode =
                parseDuplicateMode(parts[1]);
        }


        // WAIT_UNTIL_COMPLETE|true|1000|10000
        else if (parts[0] == "WAIT_UNTIL_COMPLETE")
        {
            config.actionOptions.fileStability.enabled =
                upper(parts[1]) == "TRUE";

            config.actionOptions.fileStability.stableFor =
                chrono::milliseconds(stoll(parts[2]));

            config.actionOptions.fileStability.timeout =
                chrono::milliseconds(stoll(parts[3]));
        }


        // RULE|name|event|matcher|value|action|value
        else if (parts[0] == "RULE")
        {
            string name = parts[1];

            EventType event =
                parseEvent(parts[2]);

            auto matcher =
                createMatcher(parts[3], parts[4]);

            auto action =
                createAction(
                    parts[5],
                    parts[6],
                    config.actionOptions
                );

            config.rules.emplace_back(
                name,
                event,
                move(matcher),
                move(action)
            );
        }


        else
        {
            throw runtime_error(
                "Unknown config line: " + line
            );
        }
    }

    return config;
}


// Main load function
Config ConfigManager::load(
    const filesystem::path& file) const
{
    ifstream input(file);

    if (!input)
    {
        throw runtime_error(
            "Cannot open config: " + file.string()
        );
    }

    Config config = loadConfig(input);


    // Watch directory is mandatory
    if (config.watchDirectory.empty())
    {
        throw runtime_error(
            "WATCH directory is required"
        );
    }

    return config;
}

}