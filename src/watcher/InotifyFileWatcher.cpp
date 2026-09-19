#include "watchflow/watcher/InotifyFileWatcher.hpp"

#include <sys/inotify.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace watchflow {

InotifyFileWatcher::InotifyFileWatcher(std::filesystem::path directory)
    : directory_(std::move(directory)) {}

InotifyFileWatcher::~InotifyFileWatcher() {
    stop();
}

void InotifyFileWatcher::start(EventCallback callback) {
    if (running_) return;
    running_ = true;
    worker_ = std::thread(&InotifyFileWatcher::run, this, std::move(callback));
}

void InotifyFileWatcher::stop() {
    running_ = false;
    if (worker_.joinable()) worker_.join();
}

void InotifyFileWatcher::run(EventCallback callback) {
    const int fd = inotify_init1(0);
    if (fd < 0) {
        running_ = false;
        throw std::runtime_error(std::string("inotify_init1 failed: ") + std::strerror(errno));
    }

    const int wd = inotify_add_watch(
        fd, directory_.c_str(),
        IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_TO
    );

    if (wd < 0) {
        close(fd);
        running_ = false;
        throw std::runtime_error(std::string("inotify_add_watch failed: ") + std::strerror(errno));
    }

    std::vector<char> buffer(64 * 1024);

    while (running_) {
        const ssize_t length = read(fd, buffer.data(), buffer.size());
        if (length <= 0) {
            if (errno == EINTR) continue;
            break;
        }

        ssize_t offset = 0;
        while (offset < length) {
            auto* event = reinterpret_cast<inotify_event*>(buffer.data() + offset);

            if (event->len > 0 && !(event->mask & IN_ISDIR)) {
                EventType type;
                if (event->mask & IN_CREATE) type = EventType::Created;
                else if (event->mask & IN_MODIFY) type = EventType::Modified;
                else if (event->mask & IN_DELETE) type = EventType::Deleted;
                else if (event->mask & IN_MOVED_TO) type = EventType::Moved;
                else {
                    offset += sizeof(inotify_event) + event->len;
                    continue;
                }

                callback(FileEvent{type, directory_ / event->name});
            }

            offset += sizeof(inotify_event) + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}

} // namespace watchflow
