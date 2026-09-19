# WatchFlow

A lightweight Linux file monitoring and automation engine built with modern **C++20**.

WatchFlow watches a directory for filesystem events via Linux `inotify`, evaluates each event against a set of configurable rules, and automatically triggers an action when a rule matches — copy a file, run a shell command, compress it, or fire a desktop notification.

The project doubles as a practical exercise in modern C++, Linux systems programming, OOP, SOLID principles, and event-driven design.

```
Linux Filesystem → inotify → FileWatcher → FileEvent → RuleEngine → Matcher → Action
```

---

## Table of Contents

- [Problem Statement](#problem-statement)
- [Overview](#overview)
- [Features](#features)
- [Technology Stack](#technology-stack)
- [Requirements](#requirements)
- [Installation](#installation)
- [Running WatchFlow](#running-watchflow)
- [Quick Start](#quick-start)
- [Configuration](#configuration)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Design Principles](#design-principles)
- [Security Considerations](#security-considerations)
- [Learning Goals](#learning-goals)
- [License](#license)

---

## Problem Statement

Teams and individuals often download, create, or receive files into a shared folder and then spend time manually sorting them, copying them into destination folders, launching follow-up commands, compressing archives, or sending notifications. WatchFlow automates that response loop: it watches a folder, matches files against rules, and immediately applies the right action.

---

## Overview

WatchFlow continuously monitors a directory and reacts to filesystem events. The core pipeline:

```
New file:  report.pdf
     ↓
Regex Matcher: .*\.pdf
     ↓
MATCH
     ↓
CopyAction
     ↓
output/pdfs/report.pdf
```

---

## Features

### 🔎 Filesystem Monitoring
Watches a directory for **creation**, **modification**, **deletion**, and **move** events using Linux's `inotify` — no polling.

### 📥 Automatic File Storage
When a new file appears in the watched directory, WatchFlow can automatically store it in a target folder through a `COPY` rule. This is the feature behind the "download it and it gets stored automatically" behavior: the file lands in the watch folder, WatchFlow detects it, and the configured action moves or copies it to the destination folder without manual handling.

### 🧩 Regex Matcher
Match files by pattern:

```
.*\.pdf
```
✅ `report.pdf`, `resume.pdf`   ❌ `image.png`, `main.cpp`

### 📦 File Size Matcher
Match files by size with `SIZE_GT`, `SIZE_LT`, or `SIZE_EQ`:

```
SIZE_GT|1048576    # file size > 1 MB
```

### ⚡ Actions

| Action | Description | Example |
|---|---|---|
| **COPY** | Copies the file using `std::filesystem` | `COPY\|./output/pdfs` |
| **EXECUTE** | Runs a shell command with `{file}` substitution | `EXECUTE\|g++ "{file}" -std=c++20 -o "{file}.out"` |
| **COMPRESS** | Compresses the file with `gzip` | `COMPRESS\|.gz` |
| **NOTIFY** | Sends a Linux desktop notification | `NOTIFY\|New PDF detected: {file}` |

> ⚠️ `EXECUTE` can run arbitrary shell commands — only use trusted configuration files.

---

## Technology Stack

| Technology | Purpose |
|---|---|
| C++20 | Core implementation |
| Linux | Operating system |
| inotify | Filesystem event monitoring |
| `<filesystem>` | File operations |
| `<regex>` | Regex matching |
| CMake | Build system |
| gzip | File compression |
| notify-send | Desktop notifications |

---

## Requirements

- Linux
- GCC or Clang with C++20 support
- CMake 3.20+
- `gzip`
- `libnotify` / `notify-send`

**Ubuntu / Kubuntu / Debian:**

```bash
sudo apt update
sudo apt install build-essential cmake gzip libnotify-bin
```

Verify:

```bash
g++ --version
cmake --version
gzip --version
notify-send --version
```

---

## Installation

```bash
git clone https://github.com/<your-username>/WatchFlow.git
cd WatchFlow

cmake -S . -B build
cmake --build build
```

The executable is generated at `build/watchflow`.

---

## Running WatchFlow

```bash
./build/watchflow <config-file>
```

Example:

```bash
./build/watchflow examples/watchflow.conf
```

```
====================================
          WatchFlow
====================================

Watching: ./sandbox
Rules: 4

WatchFlow started.
Press Ctrl+C to stop.
```

Stop with `Ctrl+C`.

---

## Quick Start

```bash
mkdir -p sandbox
mkdir -p output/pdfs

./build/watchflow examples/watchflow.conf
```

In another terminal:

```bash
touch sandbox/report.pdf
```

WatchFlow detects the event, matches it against `.*\.pdf`, and copies it:

```bash
ls output/pdfs
# report.pdf
```

---

## Configuration

WatchFlow uses a simple pipe-delimited configuration format.

```
WATCH|./sandbox

RULE|Copy PDFs|CREATED|REGEX|.*\.pdf|COPY|./output/pdfs
RULE|Compress Large Files|MODIFIED|SIZE_GT|1048576|COMPRESS|.gz
RULE|Compile C++|MODIFIED|REGEX|.*\.cpp|EXECUTE|g++ "{file}" -std=c++20 -o "{file}.out"
RULE|Notify PDFs|CREATED|REGEX|.*\.pdf|NOTIFY|New PDF detected: {file}
```

### `WATCH`

```
WATCH|<directory>
```

### `RULE`

```
RULE|<name>|<event>|<matcher>|<pattern>|<action>|<action-argument>
```

**Supported events:** `CREATED`, `MODIFIED`, `DELETED`, `MOVED`

**Supported matchers:**

| Matcher | Syntax |
|---|---|
| Regex | `REGEX\|<pattern>` |
| Size greater than | `SIZE_GT\|<bytes>` |
| Size less than | `SIZE_LT\|<bytes>` |
| Size equal to | `SIZE_EQ\|<bytes>` |

**Supported actions:**

| Action | Syntax |
|---|---|
| Copy | `COPY\|<destination-directory>` |
| Execute | `EXECUTE\|<command with {file}>` |
| Compress | `COMPRESS\|<extension>` |
| Notify | `NOTIFY\|<message with {file}>` |

---

## Architecture

WatchFlow follows a layered, event-driven architecture:

```
Config → ConfigManager → RuleEngine
                              ▲
Linux Kernel → inotify → InotifyFileWatcher → FileEvent
                              │
                          RuleEngine
                              │
                          IMatcher ──► RegexMatcher / FileSizeMatcher
                              │
                            MATCH
                              │
                           IAction ──► CopyAction / ExecuteAction / CompressAction / NotificationAction
```

### Runtime flow — `touch sandbox/report.pdf`

### Runtime outcome — file lands in a destination folder

If the watched folder receives a file such as `report.pdf`, the matching `COPY` rule stores it in the configured destination directory, for example `output/pdfs/report.pdf`.

1. User creates `report.pdf`
2. Linux filesystem changes
3. `inotify` generates an event
4. `InotifyFileWatcher` receives it
5. WatchFlow builds a `FileEvent`
6. `RuleEngine` receives the `FileEvent`
7. `RuleEngine` iterates through rules
8. The matching `IMatcher` evaluates the event
9. On match, the configured `IAction` executes
10. `Logger` reports the result

### Core components

| Component | Responsibility |
|---|---|
| `FileEvent` | Internal representation of a filesystem event, decoupled from `inotify_event` |
| `IFileWatcher` / `InotifyFileWatcher` | Abstraction over filesystem monitoring; Linux `inotify` implementation |
| `IMatcher` / `RegexMatcher` / `FileSizeMatcher` | Strategy interface for evaluating events against rule conditions |
| `IAction` / `CopyAction` / `ExecuteAction` / `CompressAction` / `NotificationAction` | Independently executable action objects |
| `Rule` | Binds an event type, a matcher, and an action together |
| `RuleEngine` | Central coordinator: receives events, checks rules, dispatches actions |
| `ConfigManager` | Parses the config file and builds the rule model |

---

## Project Structure

```
WatchFlow/
├── CMakeLists.txt
├── README.md
├── examples/
│   └── watchflow.conf
├── docs/
│   └── INSTRUCTIONS.md
├── include/watchflow/
│   ├── core/        FileEvent.hpp
│   ├── watcher/      IFileWatcher.hpp, InotifyFileWatcher.hpp
│   ├── matcher/       IMatcher.hpp, RegexMatcher.hpp, FileSizeMatcher.hpp
│   ├── action/        IAction.hpp, CopyAction.hpp, ExecuteAction.hpp, CompressAction.hpp, NotificationAction.hpp
│   ├── rule/          Rule.hpp, RuleEngine.hpp
│   ├── config/        ConfigManager.hpp
│   └── logging/       ConsoleLogger.hpp
├── src/                (mirrors include/ + main.cpp)
└── tests/
    └── README.md
```

---

## Design Principles

### OOP
- **Encapsulation** — each class owns its data and behavior (e.g. `RegexMatcher` owns its pattern + matching logic)
- **Abstraction** — Linux specifics hidden behind `IFileWatcher`, matching behind `IMatcher`, actions behind `IAction`
- **Polymorphism** — `RuleEngine` operates on `IMatcher`/`IAction` without knowing the concrete type

### SOLID
| Principle | Applied as |
|---|---|
| **S**RP | Each class has one job — `InotifyFileWatcher` watches, `ConfigManager` parses, `RuleEngine` evaluates |
| **O**CP | New matchers/actions can be added without modifying `RuleEngine` |
| **L**SP | Any `IMatcher`/`IAction` implementation is substitutable wherever the interface is expected |
| **I**SP | Focused interfaces — `IFileWatcher`, `IMatcher`, `IAction` — instead of one large interface |
| **D**IP | `RuleEngine` depends on `IMatcher`/`IAction` abstractions, not concrete classes |

### Design Patterns
- **Strategy** — pluggable matching algorithms (`RegexMatcher`, `FileSizeMatcher`)
- **Command-style actions** — each action is an independent, executable object
- **Factory-style creation** — `ConfigManager` maps config tokens (`REGEX`, `COPY`, …) to concrete objects
- **Observer-style events** — `InotifyFileWatcher` emits `FileEvent`s consumed by `RuleEngine`

---

## Security Considerations

The `EXECUTE` action can run **any** shell command available to the user running WatchFlow.

- ❌ Never run untrusted configuration files
- ⚠️ Be careful with `{file}` substitution when constructing commands
- ✅ Intended for personal automation, dev environments, and controlled Linux setups
- 🚫 Not a hardened, multi-user automation service

---

## Current Limitations

This is an MVP. It does **not** yet support:

- Recursive directory watching
- YAML/JSON configuration
- Database storage, web UI, or GUI
- Authentication
- Worker pool / concurrent execution
- Job persistence, retries, or scheduling
- File locking
- Sandboxing
- Cross-platform monitoring (Windows/macOS)

---

## Roadmap

| Version | Focus |
|---|---|
| **v0.2** | Better config format, more matchers/actions |
| **v0.3** | Recursive watching, better error handling & logging |
| **v0.4** | Worker thread pool, event queue, concurrent actions |
| **v0.5** | Retry mechanism, action status, timeouts, job management |
| **v1.0** | Production CLI, config validation, persistent logs, test suite, daemon mode |

---

## Learning Goals

- **Linux:** `inotify`, filesystem events, processes, shell commands
- **C++:** C++20, RAII, smart pointers, STL, `<filesystem>`, `<regex>`, interfaces, polymorphism, exceptions
- **OOP:** Encapsulation, Abstraction, Inheritance, Polymorphism, Composition
- **SOLID:** SRP, OCP, LSP, ISP, DIP
- **Design Patterns:** Strategy, Command-style, Factory-style, Observer-style

---

## License

MIT License — see `LICENSE` for details.