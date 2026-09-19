# WatchFlow — Build Instructions

## 1. Goal

Build a small Linux-only C++20 event-driven automation engine.

The core flow is:

```text
Filesystem
   |
   v
inotify
   |
   v
FileEvent
   |
   v
RuleEngine
   |
   +--> Matcher
   |
   +--> Action
```

## 2. Core feature scope

### Watcher
- Watch one directory.
- Detect CREATED, MODIFIED, DELETED, MOVED_TO.
- Ignore directories.

### Matchers
Only:
- RegexMatcher
- FileSizeMatcher

### Actions
Only:
- CopyAction
- ExecuteAction
- CompressAction
- NotificationAction

### Configuration
Use the simple `.conf` format.

The configuration format is the simple `.conf` file used by the project.

## 3. OOP/SOLID rules

Keep these interfaces:

```text
IFileWatcher
IMatcher
IAction
```

Responsibilities:

- watcher = filesystem events
- matcher = event matching
- action = side effect
- rule = combines event type + matcher + action
- rule engine = evaluates rules

Do not make one mega class.

## 4. Design patterns

### Strategy
`IMatcher` is the strategy interface.

```text
IMatcher
  +-- RegexMatcher
  +-- FileSizeMatcher
```

### Command
`IAction` represents an executable operation.

```text
IAction
  +-- CopyAction
  +-- ExecuteAction
  +-- CompressAction
  +-- NotificationAction
```

### Factory
`ConfigManager` creates concrete matcher/action objects from configuration.

This is intentionally kept as simple factory logic. A dedicated `MatcherFactory` / `ActionFactory` can be extracted later if the number of types grows.

### Observer-style event flow
The watcher publishes each `FileEvent` through a callback. The callback sends the event to the logger and rule engine. We avoid a full observer hierarchy to keep the code small.

## 5. Build order

Do not implement everything at once.

### Step 1
Make `inotify` print filesystem events.

### Step 2
Create `FileEvent`.

### Step 3
Introduce `IFileWatcher` and `InotifyFileWatcher`.

### Step 4
Implement `RegexMatcher`.

### Step 5
Implement `FileSizeMatcher`.

### Step 6
Implement `IAction`.

### Step 7
Implement Copy, Execute, Compress and Notification.

### Step 8
Implement `Rule` and `RuleEngine`.

### Step 9
Implement the simple config parser.

### Step 10
Connect everything in `main.cpp`.

## 6. Testing checklist

Start WatchFlow:

```bash
./build/watchflow examples/watchflow.conf
```

Then test:

### Regex + Copy

```bash
cp sample.pdf sandbox/
```

Expected:

```text
[RULE] Copy PDFs
[SUCCESS]
```

### Regex + Execute

```bash
echo 'int main() { return 0; }' > sandbox/test.cpp
```

Then modify it.

Expected command:

```text
g++ ".../test.cpp" -std=c++20 -o ".../test.cpp.out"
```

### Size + Compress

Create a file larger than 1 MB:

```bash
dd if=/dev/zero of=sandbox/large.bin bs=1M count=2
```

Modify it.

Expected:

```text
large.bin.gz
```

### Notification

Create a PDF and confirm the Linux desktop notification appears.

## 7. Project notes

WatchFlow is a file automation project that watches a directory, evaluates each event against rules, and stores or processes matching files automatically. The same config can be used to copy downloads into a destination folder, run follow-up commands, compress files, or send notifications.
