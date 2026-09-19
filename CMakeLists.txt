cmake_minimum_required(VERSION 3.20)
project(WatchFlow VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(watchflow
    src/main.cpp
    src/core/FileEvent.cpp
    src/watcher/InotifyFileWatcher.cpp
    src/matcher/RegexMatcher.cpp
    src/matcher/FileSizeMatcher.cpp
    src/action/CopyAction.cpp
    src/action/FileActionUtils.cpp
    src/action/MoveAction.cpp
    src/action/ExecuteAction.cpp
    src/action/CompressAction.cpp
    src/action/NotificationAction.cpp
    src/rule/Rule.cpp
    src/rule/RuleEngine.cpp
    src/config/ConfigManager.cpp
    src/logging/ConsoleLogger.cpp
    src/logging/FileLogger.cpp
    src/matcher/ExtensionMatcher.cpp
)

target_include_directories(watchflow PRIVATE include)

target_compile_options(watchflow PRIVATE
    -Wall -Wextra -Wpedantic
)
