#include "watchflow/action/CompressAction.hpp"
#include <cstdlib>
#include <filesystem>

using namespace std;

namespace watchflow {


// Put single quotes around a file path
string shellQuote(string value)
{
    return "'" + value + "'";
}


// Constructor
CompressAction::CompressAction(string outputSuffix)
{
    outputSuffix_ = outputSuffix;
}


// Compress the file
bool CompressAction::execute(const FileEvent& event)
{
    // Check if file exists
    if (!filesystem::exists(event.path))
        return false;

    // Create output filename
    string output =
        event.path.string() + outputSuffix_;

    // Create gzip command
    string command =
        "gzip -c " +
        shellQuote(event.path.string()) +
        " > " +
        shellQuote(output);

    // Execute command
    int result = system(command.c_str());

    return result == 0;
}

}