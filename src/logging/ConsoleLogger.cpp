#include "watchflow/logging/ConsoleLogger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

using namespace std;

namespace watchflow {

void ConsoleLogger::log(const FileEvent& event) const
{
    // Get current time
    auto now = chrono::system_clock::now();

    // Convert time to time_t
    time_t currentTime = chrono::system_clock::to_time_t(now);

    // Print timestamp
    cout << "["
         << put_time(localtime(&currentTime), "%H:%M:%S")
         << "] ";

    // Print event type
    cout << toString(event.type) << " ";

    // Print file path
    cout << event.path << "\n";
}

}