# tidesh

`tidesh` is a minimal Unix shell written in C. It was originally based on [Stephen Brennan's "Write a Shell in C"](https://brennan.io/2015/01/16/write-a-shell-in-c/), but has since been cleaned up, extended, and rebranded with slightly less shame.
## will be developed further, soon... if I don't slack off


## Features

- Basic REPL loop (`read -> parse -> execute`)
- Built-in commands:
  - `cd`
  - `help`
  - `exit`
- Executes external programs using `execvp`
- Handles `Ctrl+C` gracefully (no shell crash)
- Prompt shows current working directory

## Getting Started

### Requirements

- A POSIX-compatible system (Linux, macOS)
- C compiler (`gcc` or `clang`)

### Build

```bash
clang -o tidesh tidesh.c
# or
gcc -o tidesh tidesh.c
