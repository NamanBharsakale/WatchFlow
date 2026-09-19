#pragma once

#include "watchflow/watcher/IFileWatcher.hpp"
#include <atomic>
#include <filesystem>
#include <thread>

namespace watchflow {

class InotifyFileWatcher final : public IFileWatcher {
public:
    explicit InotifyFileWatcher(std::filesystem::path directory);
    ~InotifyFileWatcher() override;

    void start(EventCallback callback) override;
    void stop() override;

private:
    void run(EventCallback callback);

    std::filesystem::path directory_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};

} // namespace watchflow
