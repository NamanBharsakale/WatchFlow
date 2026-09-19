# WatchFlow Instructions

This document describes the current project behavior and the expected way to
build, run, and extend WatchFlow.

## Goal

WatchFlow is a Linux-only C++20 automation tool. It watches one directory,
creates a `FileEvent` for each supported filesystem event, checks configured
rules, and executes the matching action.

```text
Filesystem
  -> inotify
  -> InotifyFileWatcher
  -> FileEvent
  -> RuleEngine
  -> IMatcher
  -> IAction
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/watchflow examples/watchflow.conf
```

Stop with `Ctrl+C`.

## Core Scope

### Watcher

- Watches one configured directory.
- Uses Linux `inotify`.
- Emits events for `CREATED`, `MODIFIED`, `DELETED`, and `MOVED`.
- Ignores directories.
- Stops cleanly on `SIGINT` and `SIGTERM`.

### Matchers

- `ExtensionMatcher`: case-insensitive extension matching, with or without dots.
- `RegexMatcher`: C++ regex matching against the event path.
- `FileSizeMatcher`: `SIZE_GT`, `SIZE_LT`, and `SIZE_EQ` byte comparisons.

### Actions

- `MoveAction`: moves files into a destination directory.
- `CopyAction`: copies files into a destination directory.
- `ExecuteAction`: runs a shell command with `{file}` substitution.
- `CompressAction`: compresses with `gzip`.
- `NotificationAction`: sends desktop notifications with `notify-send`.

Move and copy actions support:

- duplicate behavior: `overwrite`, `rename`, or `skip`
- optional wait-until-complete checks before acting

### Logging

- `ConsoleLogger` prints events to stdout.
- `FileLogger` optionally appends events and rule results to `log_file`.
- Log parent directories are created automatically.

## Configuration

WatchFlow supports two config formats.

### YAML-style `.yml` / `.yaml`

Use this format for new configs.

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
```

Supported top-level keys:

- `watch`
- `log_file`
- `duplicates`
- `wait_until_complete`
- `rules`

Supported rule keys:

- `name`
- `event` or `events`
- `matcher`
- `extensions`
- `pattern`
- `value`
- `action`
- `destination`

Supported events:

- `created`
- `modified`
- `deleted`
- `moved`

Supported matchers:

- `extension`
- `regex`
- `size_gt`
- `size_lt`
- `size_eq`

Supported actions:

- `move`
- `copy`
- `execute`
- `compress`
- `notify`

Note: YAML support is intentionally small and only supports the simple shape
above. It is not a full YAML parser.

### Legacy `.conf`

Existing pipe-delimited configs are still supported.

```text
WATCH|./sandbox
LOG|./logs/watchflow.log
DUPLICATES|rename
WAIT_UNTIL_COMPLETE|true|1500|60000

RULE|Copy PDFs|CREATED|REGEX|.*\.pdf|COPY|./output/pdfs
RULE|Move PDFs|MOVED|EXT|pdf|MOVE|./output/pdfs
RULE|Compress large files|MODIFIED|SIZE_GT|1048576|COMPRESS|.gz
RULE|Compile C++|MODIFIED|REGEX|.*\.cpp|EXECUTE|g++ "{file}" -std=c++20 -o "{file}.out"
```

Rule format:

```text
RULE|name|EVENT|MATCHER|MATCHER_VALUE|ACTION|ACTION_VALUE
```

## Design Rules

Keep the current small-interface design:

- `IFileWatcher` owns filesystem event production.
- `IMatcher` owns rule matching.
- `IAction` owns side effects.
- `Rule` binds event type, matcher, and action.
- `RuleEngine` evaluates rules and dispatches actions.
- `ConfigManager` builds rules from configuration.

Do not collapse these into a single manager class.

## Patterns Used

- Strategy: `IMatcher` implementations are interchangeable.
- Command-style actions: each `IAction` can execute independently.
- Factory-style config loading: `ConfigManager` maps config tokens to concrete classes.
- Observer-style event flow: the watcher publishes events through a callback.

## Manual Testing

Build first:

```bash
cmake --build build
```

Smoke-test config parsing and shutdown:

```bash
timeout 2s ./build/watchflow examples/watchflow.conf
timeout 2s ./build/watchflow downloads-sort.yml
```

Test PDF copy with the example config:

```bash
mkdir -p sandbox output/pdfs
./build/watchflow examples/watchflow.conf
```

In another terminal:

```bash
touch sandbox/report.pdf
ls output/pdfs
```

Expected result:

```text
report.pdf
```

Test large-file compression:

```bash
dd if=/dev/zero of=sandbox/large.bin bs=1M count=2
touch sandbox/large.bin
```

Expected result:

```text
sandbox/large.bin.gz
```

Test download sorting with the YAML config:

```bash
./build/watchflow downloads-sort.yml
```

Then move or create files in `/home/naman/Downloads` and confirm they are moved
to the configured destination folders.

## Extension Notes

When adding a new matcher:

1. Add the header under `include/watchflow/matcher/`.
2. Add the implementation under `src/matcher/`.
3. Register the source file in `CMakeLists.txt`.
4. Add creation logic in `ConfigManager::makeMatcher`.
5. Document the config token in `README.md`.

When adding a new action:

1. Add the header under `include/watchflow/action/`.
2. Add the implementation under `src/action/`.
3. Register the source file in `CMakeLists.txt`.
4. Add creation logic in `ConfigManager::makeAction`.
5. Document the config token in `README.md`.

## Security Notes

`ExecuteAction` calls `std::system`. Treat any config file with `execute` or
`EXECUTE` as trusted code.

- Prefer native `move` and `copy` for sorting files.
- Quote `{file}` in shell commands.
- Do not run untrusted configs.
- Do not expose WatchFlow as a shared automation service without sandboxing.

## Known Limitations

- One watched directory per process.
- No recursive watching.
- Linux only.
- No automated test framework yet.
- No action queue or retries.
- No daemon/service installer.
