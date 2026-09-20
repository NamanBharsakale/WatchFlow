#include "watchflow/watcher/InotifyFileWatcher.hpp"

#include <sys/inotify.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace watchflow;

using std::chrono::milliseconds;
using std::move;
using std::runtime_error;
using std::string;
using std::thread;
using std::vector;

InotifyFileWatcher::InotifyFileWatcher(std::filesystem::path directory)
    : directory_(move(directory)) {
}

InotifyFileWatcher::~InotifyFileWatcher() {
    stop();
}

void InotifyFileWatcher::start(EventCallback callback) {

    // Don't start if already running
    if (running_) {
        return;
    }

    running_ = true;

    // Start watcher in a separate thread
    worker_ = thread(
        &InotifyFileWatcher::run,
        this,
        move(callback)
    );
}

void InotifyFileWatcher::stop() {

    // Tell watcher thread to stop
    running_ = false;

    // Wait for watcher thread to finish
    if (worker_.joinable()) {
        worker_.join();
    }
}

void InotifyFileWatcher::run(EventCallback callback) {

    // Create inotify instance
    const int fd = inotify_init1(IN_NONBLOCK);

    if (fd < 0) {
        running_ = false;

        throw runtime_error(
            string("inotify_init1 failed: ") +
            std::strerror(errno)
        );
    }

    // Watch the configured directory
    const int wd = inotify_add_watch(
        fd,
        directory_.c_str(),
        IN_CREATE |
        IN_MODIFY |
        IN_DELETE |
        IN_MOVED_TO
    );

    if (wd < 0) {
        close(fd);
        running_ = false;

        throw runtime_error(
            string("inotify_add_watch failed: ") +
            std::strerror(errno)
        );
    }

    // Buffer for filesystem events
    vector<char> buffer(64 * 1024);

    // Keep watching while running
    while (running_) {

        // Read events from inotify
        const ssize_t length =
            read(fd, buffer.data(), buffer.size());

        // No event or read error
        if (length <= 0) {

            // Interrupted by signal
            if (errno == EINTR) {
                continue;
            }

            // No event available
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(milliseconds(100));
                continue;
            }

            // Other error
            break;
        }

        // Process all events in the buffer
        ssize_t offset = 0;

        while (offset < length) {

            auto* event =
                reinterpret_cast<inotify_event*>(
                    buffer.data() + offset
                );

            // Ignore directories
            if (event->len > 0 &&
                !(event->mask & IN_ISDIR)) {

                EventType type;

                // Find event type
                if (event->mask & IN_CREATE) {
                    type = EventType::Created;
                }
                else if (event->mask & IN_MODIFY) {
                    type = EventType::Modified;
                }
                else if (event->mask & IN_DELETE) {
                    type = EventType::Deleted;
                }
                else if (event->mask & IN_MOVED_TO) {
                    type = EventType::Moved;
                }
                else {
                    offset += sizeof(inotify_event) + event->len;
                    continue;
                }

                // Create WatchFlow event
                FileEvent fileEvent{
                    type,
                    directory_ / event->name
                };

                // Send event to callback
                callback(fileEvent);
            }

            // Move to next event
            offset += sizeof(inotify_event) + event->len;
        }
    }

    // Cleanup
    inotify_rm_watch(fd, wd);
    close(fd);
}