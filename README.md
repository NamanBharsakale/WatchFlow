# WatchFlow

WatchFlow is a lightweight Linux file watcher and automation engine written in
C++20. It watches one directory with `inotify`, turns filesystem changes into
events, matches those events against configurable rules, and runs actions such
as moving downloads, copying files, compressing large files, running commands,
or sending desktop notifications.

```text
Linux filesystem -> inotify -> FileEvent -> RuleEngine -> Matcher -> Action
```

## Features

- Linux `inotify` monitoring for created, modified, deleted, and moved-in files.
- Rule-based automation with one or more rules per config file.
- YAML-style configuration for readable download sorting rules.
- Legacy pipe-delimited `.conf` configuration support.
- Matchers for file extensions, regular expressions, and file size.
- Actions for move, copy, execute, gzip compression, and desktop notification.
- Duplicate handling for move/copy actions: overwrite, rename, or skip.
- Optional wait-until-complete behavior before moving or copying files.
- Console logging for events and rule results.
- Optional file logging with automatic log directory creation.
- Graceful shutdown on `Ctrl+C` or `SIGTERM`.

## Requirements

- Linux
- GCC or Clang with C++20 support
- CMake 3.20 or newer
- `gzip` for the `compress` action
- `notify-send` from `libnotify-bin` for the `notify` action

On Debian, Ubuntu, or Kubuntu:

```bash
sudo apt update
sudo apt install build-essential cmake gzip libnotify-bin
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

The executable is created at:

```bash
./build/watchflow
```

## Run

```bash
./build/watchflow <config-file>
```

Examples:

```bash
./build/watchflow examples/watchflow.conf
./build/watchflow downloads-sort.yml
```

Stop WatchFlow with `Ctrl+C`.

## Quick Start

Create the local demo folders:

```bash
mkdir -p sandbox output/pdfs
```

Start WatchFlow:

```bash
./build/watchflow examples/watchflow.conf
```

In another terminal, create a PDF:

```bash
touch sandbox/report.pdf
```

The demo rule copies it to:

```text
output/pdfs/report.pdf
```

## YAML Configuration

YAML files use the `.yml` or `.yaml` extension. WatchFlow supports a small,
purpose-built YAML subset: top-level key/value fields, `wait_until_complete`,
and a list of simple rule objects.

```yaml
watch: /home/naman/Downloads
log_file: /home/naman/Music/WatchFlow/logs/downloads.log
duplicates: rename

wait_until_complete:
  enabled: true
  stable_for_ms: 1500
  timeout_ms: 60000

rules:
  - name: Images
    events: [created, moved]
    matcher: extension
    extensions: [jpg, jpeg, png, gif, webp, bmp, svg]
    action: move
    destination: /home/naman/Pictures

  - name: PDFs
    events: [created, moved]
    matcher: extension
    extensions: [pdf]
    action: move
    destination: /home/naman/Documents/PDF
```

### Top-Level Fields

| Field | Required | Description |
| --- | --- | --- |
| `watch` | Yes | Directory to monitor. |
| `log_file` | No | File that receives event and rule-result logs. Parent directories are created automatically. |
| `duplicates` | No | Move/copy behavior when the destination file exists: `overwrite`, `rename`, or `skip`. Default: `overwrite`. |

### Wait Until Complete

`wait_until_complete` applies to native `move` and `copy` actions. When enabled,
WatchFlow waits until the source file size remains stable before acting. This is
useful for browser downloads and large copied files.

| Field | Description |
| --- | --- |
| `enabled` | `true` or `false`. Default: `false`. |
| `stable_for_ms` | How long the file size must stay unchanged. Default: `1000`. |
| `timeout_ms` | Maximum time to wait before the action fails. Default: `60000`. |

### Rule Fields

| Field | Description |
| --- | --- |
| `name` | Human-readable rule name. |
| `event` | One event: `created`, `modified`, `deleted`, or `moved`. |
| `events` | List of events. If omitted, defaults to `[created, moved]`. |
| `matcher` | `extension`, `regex`, `size_gt`, `size_lt`, or `size_eq`. |
| `extensions` | Extension list for `extension` matcher. Dots are optional and matching is case-insensitive. |
| `pattern` | Regex pattern for `regex` matcher. |
| `value` | Generic value used by size matchers or as an action fallback. |
| `action` | `move`, `copy`, `execute`, `compress`, or `notify`. |
| `destination` | Destination directory for `move` and `copy`. Also used as the action argument when present. |

For size matchers, use `value` as the byte threshold:

```yaml
rules:
  - name: Large files
    event: modified
    matcher: size_gt
    value: 1048576
    action: compress
    destination: .gz
```

## Legacy `.conf` Configuration

Pipe-delimited config files are still supported.

```text
WATCH|./sandbox
LOG|./logs/watchflow.log
DUPLICATES|rename
WAIT_UNTIL_COMPLETE|true|1500|60000

RULE|Copy PDFs|CREATED|REGEX|.*\.pdf|COPY|./output/pdfs
RULE|Move PDFs|MOVED|EXT|pdf|MOVE|./output/pdfs
RULE|Compress large files|MODIFIED|SIZE_GT|1048576|COMPRESS|.gz
RULE|Compile C++|MODIFIED|REGEX|.*\.cpp|EXECUTE|g++ "{file}" -std=c++20 -o "{file}.out"
RULE|Notify PDFs|CREATED|REGEX|.*\.pdf|NOTIFY|New PDF: {file}
```

Supported rule format:

```text
RULE|<name>|<event>|<matcher>|<matcher-value>|<action>|<action-value>
```

Supported events:

```text
CREATED, MODIFIED, DELETED, MOVED
```

Supported matchers:

| Matcher | Value |
| --- | --- |
| `EXT` / `EXTENSION` | Comma-separated extensions, such as `pdf,jpg,png`. |
| `REGEX` | C++ regex pattern matched against the event path. |
| `SIZE_GT` | File size is greater than the given byte count. |
| `SIZE_LT` | File size is less than the given byte count. |
| `SIZE_EQ` | File size equals the given byte count. |

Supported actions:

| Action | Value |
| --- | --- |
| `MOVE` | Destination directory. |
| `COPY` | Destination directory. |
| `EXECUTE` | Shell command. `{file}` is replaced with the event path. |
| `COMPRESS` | Output extension, usually `.gz`. |
| `NOTIFY` | Desktop notification message. `{file}` is replaced with the event path. |

## Download Sorting

The included `downloads-sort.yml` is the recommended config for sorting common
download types:

- images to `~/Pictures`
- PDFs to `~/Documents/PDF`
- Microsoft Office files to `~/Documents/MSFT`
- MP3 files to `~/Music`
- MP4 files to `~/Videos`

Run it with:

```bash
./build/watchflow downloads-sort.yml
```

Adjust the absolute paths before using it on another machine.

## Architecture

WatchFlow is split into small components:

| Component | Responsibility |
| --- | --- |
| `FileEvent` | Internal event type and path model. |
| `IFileWatcher` / `InotifyFileWatcher` | Linux filesystem monitoring. |
| `IMatcher` | Matcher strategy interface. |
| `ExtensionMatcher` | Case-insensitive file extension matching. |
| `RegexMatcher` | Regex matching against event paths. |
| `FileSizeMatcher` | File size comparisons. |
| `IAction` | Action command interface. |
| `MoveAction` | Moves files with duplicate and stability options. |
| `CopyAction` | Copies files with duplicate and stability options. |
| `ExecuteAction` | Runs shell commands with `{file}` substitution. |
| `CompressAction` | Runs `gzip` for file compression. |
| `NotificationAction` | Runs `notify-send` for desktop notifications. |
| `Rule` | Binds one event type, matcher, and action. |
| `RuleEngine` | Evaluates events and dispatches matching actions. |
| `ConfigManager` | Parses `.yml`, `.yaml`, and `.conf` configs. |
| `ConsoleLogger` / `FileLogger` | Event and rule-result logging. |

## Project Structure

```text
WatchFlow/
|-- CMakeLists.txt
|-- README.md
|-- downloads-sort.yml
|-- downloads-sort.conf
|-- docs/
|   `-- INSTRUCTIONS.md
|-- examples/
|   `-- watchflow.conf
|-- include/watchflow/
|   |-- action/
|   |-- config/
|   |-- core/
|   |-- logging/
|   |-- matcher/
|   |-- rule/
|   `-- watcher/
|-- src/
|   |-- action/
|   |-- config/
|   |-- core/
|   |-- logging/
|   |-- matcher/
|   |-- rule/
|   |-- watcher/
|   `-- main.cpp
`-- tests/
    `-- README.md
```

## Security Notes

`EXECUTE` uses `std::system`, so it can run any command available to the user
running WatchFlow.

- Only run trusted config files.
- Quote `{file}` in shell commands when paths may contain spaces.
- Do not use WatchFlow as a hardened multi-user automation service.
- Prefer native `move` and `copy` actions over shell commands for file sorting.

## Current Limitations

- Watches one directory only; no recursive watching yet.
- Linux only.
- YAML support is a small supported subset, not a general YAML parser.
- Actions run synchronously in the watcher callback.
- No retry queue, persistent job database, daemon installer, GUI, or web UI.
- No automated test suite yet; current checks are build and manual/integration smoke tests.

## Development Check

```bash
cmake --build build
timeout 2s ./build/watchflow examples/watchflow.conf
timeout 2s ./build/watchflow downloads-sort.yml
```

The timeout commands are smoke checks: they verify that config files parse,
startup works, and signal shutdown does not hang.

## License

MIT License.
