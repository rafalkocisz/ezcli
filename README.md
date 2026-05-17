# ezcli

A minimal, dependency-free C++ command line argument parsing library. Single header +
source pair, Orthodox C++ style, ASCII-only, no Unicode.

Part of the **ez** library family — shared design principles and conventions in [EZ.md](ez/EZ.md).

## Features

- Define CLI interface via `ez::CLIConfig` — flags, value options, positional arguments
- Parse `argc`/`argv` with a single `ez::cli_parse()` call
- Short flags (`-v`), long flags (`--verbose`), or both combined
- Short flag clustering: `-abc` equivalent to `-a -b -c`
- Short value options: `-o file` or `-ofile` (packed, no space)
- Long value options: `--output file` or `--output=file`
- Named positional arguments; optional variadic last positional
- `--` end-of-options separator
- `--help`, `--usage`, `--version` built-in — pre-formatted strings returned to caller
- Specific error codes — distinguish config errors from user input errors
- Never prints — all output returned as strings or codes; caller decides what to display
- `std::vector` used for config storage and parse results — startup-only allocation,
  not performance-sensitive

**Not supported:** subcommands, optional option arguments, prefix/abbreviation matching,
`--man`, Unicode.

## Quick start

```cpp
#include "ez_cli.h"
#include <cstdio>
#include <string>

int main(int argc, const char* const* argv)
{
    // Define the CLI interface
    ez::CLIConfig config;
    config.set_version("1.0.0");
    int r;
    r = config.add_flag("verbose", 'v', "Enable verbose output");     assert(r == EZ_CLI_OK);
    r = config.add_flag("dry-run", '\0', "Simulate without writing"); assert(r == EZ_CLI_OK);
    r = config.add_option("output", 'o', "Output file path");         assert(r == EZ_CLI_OK);
    r = config.add_positional("input", "Input file to process");      assert(r == EZ_CLI_OK);

    // Parse
    ez::CLIFlags   flags;
    ez::CLIOptions options;
    ez::CLIArgs    args;
    std::string    message;

    int result = ez::cli_parse(argc, argv, config, &flags, &options, &args, &message);

    if (result == EZ_CLI_HELP_REQUESTED ||
        result == EZ_CLI_USAGE_REQUESTED ||
        result == EZ_CLI_VERSION_REQUESTED) {
        printf("%s\n", message.c_str());
        return 0;
    }

    if (result != EZ_CLI_OK) {
        printf("error: %s\n", message.c_str());
        return 1;
    }

    // Query results
    if (flags.is_set("verbose"))
        printf("verbose mode on\n");

    if (options.is_set("output"))
        printf("output: %.*s\n", (int)options.get("output").size(),
                                      options.get("output").data());

    printf("input: %.*s\n", (int)args.get("input").size(),
                                  args.get("input").data());
    return 0;
}
```

## API

### `ez::CLIConfig`

```cpp
int add_flag(const char* long_name, char short_name, const char* description);
int add_option(const char* long_name, char short_name, const char* description);
int add_positional(const char* name, const char* description);
int add_positional_list(const char* name, const char* description);
```

Pass `nullptr` for `long_name` to define a short-only flag/option; pass `'\0'` for
`short_name` to define a long-only flag/option. `add_positional_list()` must be called
last and only once — asserted. Return values are config error codes; **always assert on
them** — config errors are programmer errors, not runtime conditions:

```cpp
int r = config.add_flag("verbose", 'v', "Enable verbose output");
assert(r == EZ_CLI_OK);
```

### `ez::cli_parse`

```cpp
int cli_parse(int argc, const char* const* argv,
              const CLIConfig& config,
              CLIFlags*  flags   = nullptr,
              CLIOptions* options = nullptr,
              CLIArgs*   args    = nullptr,
              std::string* message = nullptr);
```

| Parameter | Description |
|-----------|-------------|
| `argc`, `argv` | Passed directly from `main()` |
| `config` | Defines the expected CLI interface |
| `flags` | If non-null, filled with parsed flags on success; **always cleared** on entry |
| `options` | If non-null, filled with parsed options on success; **always cleared** on entry |
| `args` | If non-null, filled with parsed positional arguments on success; **always cleared** on entry |
| `message` | If non-null, filled with error message on failure or pre-formatted help/usage/version string on meta-option request; **always cleared** on entry |

String views in `flags`, `options`, and `args` are valid as long as `argv` is alive.

### Return values

| Code | Value | Meaning |
|------|------:|---------|
| `EZ_CLI_OK` | `0` | Successful parse |
| `EZ_CLI_HELP_REQUESTED` | `1` | `--help` detected; `message` filled |
| `EZ_CLI_USAGE_REQUESTED` | `2` | `--usage` detected; `message` filled |
| `EZ_CLI_VERSION_REQUESTED` | `3` | `--version` detected; `message` filled |
| `EZ_CLI_NO_MATCH` | `4` | Unrecognised flag or option |
| `EZ_CLI_ERR_SYNTAX` | `-1` | Generic catch-all (reserved) |
| `EZ_CLI_ERR_CONFIG_NAME` | `-2` | Invalid or duplicate flag/option/positional name |
| `EZ_CLI_ERR_MISSING_VAL` | `-3` | Option requires a value but none was provided |
| `EZ_CLI_ERR_EMPTY_VAL` | `-4` | `--option=` with empty value after `=` |

Any negative return value means a runtime error; test with `result < 0` to catch all
errors. Config errors (`EZ_CLI_ERR_CONFIG_*`) should be asserted away during development.

### `ez::CLIFlags`

```cpp
bool is_set(const char* name) const;  // name is long or short name
```

### `ez::CLIOptions`

```cpp
bool             is_set(const char* name) const;
std::string_view get(const char* name) const;  // empty if not provided
```

### `ez::CLIArgs`

```cpp
std::string_view              get(const char* name) const;       // named positional
std::vector<std::string_view> get_list(const char* name) const;  // variadic positional
```

## Supported CLI conventions

| Convention | Example |
|------------|---------|
| Short flag | `-v`, `-Z` |
| Short flag clustering | `-abc` = `-a -b -c` |
| Short value option, space | `-o file.txt` |
| Short value option, packed | `-ofile.txt` |
| Short cluster + value option last | `-abco file.txt`, `-abcofile.txt` |
| Long flag | `--verbose`, `--dry-run` |
| Long value option, space | `--output file.txt` |
| Long value option, `=` | `--output=file.txt` |
| Short + long combined | `-v` / `--verbose` |
| End-of-options separator | `--` |
| Built-in meta-options | `--help`, `--usage`, `--version` |

## Build

**Requirements:** CMake ≥ 3.20, a C++17 compiler.

### Linux / macOS

```sh
cmake -B build
cmake --build build
```

### Windows (Visual Studio)

```bat
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

To build without tests:

```sh
cmake -B build -DEZCLI_BUILD_TESTS=OFF
cmake --build build
```

## Sanitizers

Running the test suite under sanitizers is the recommended way to check for memory errors
and undefined behaviour.

| Sanitizer | MSVC (VS 2022) | GCC / Clang |
|---|---|---|
| ASan (Address Sanitizer) | `/fsanitize=address` | `-fsanitize=address` |
| UBSan (Undefined Behaviour) | not supported | `-fsanitize=undefined` |

### Build

Use a **separate build directory** to keep the sanitized build isolated from the normal one.

```sh
# Linux / macOS — ASan + UBSan
cmake -B build-san -DEZCLI_SANITIZE=ON
cmake --build build-san

# Windows (ASan only — MSVC does not support UBSan)
cmake -B build-san -G "Visual Studio 17 2022" -DEZCLI_SANITIZE=ON
cmake --build build-san --config RelWithDebInfo
```

`RelWithDebInfo` is recommended on MSVC: it gives optimised code with debug symbols, which
produces the most actionable ASan stack traces.

### Run

```sh
# Linux / macOS
./build-san/tests/test_ez_cli

# Windows — the ASan runtime DLL must be on PATH before running.
# Adjust the MSVC version number to match your installation.
$asanDir = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x64"
$env:PATH = "$asanDir;$env:PATH"
.\build-san\tests\RelWithDebInfo\test_ez_cli.exe
```

> **MSVC notes:**
> - The `/RTC1` runtime-check flag (added by CMake to Debug builds) is incompatible with
>   ASan; the CMakeLists.txt strips it automatically when `EZCLI_SANITIZE=ON`.
> - The `/INCREMENTAL` linker option is likewise removed automatically.
> - Use `RelWithDebInfo` rather than `Debug` for the sanitized build.

### For UBSan on Windows

The easiest option is WSL2 with GCC or Clang:

```sh
# Inside WSL2 (Ubuntu)
cmake -B build-san -DEZCLI_SANITIZE=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-san
./build-san/tests/test_ez_cli
```

### What the sanitizers check

All 137 tests pass clean under both ASan (MSVC, Windows) and ASan + UBSan (Clang, WSL2)
with no reported errors.

## Fuzzing

**Requirements:** Clang, ASan, UBSan. Supported on Linux and WSL2.

```sh
cmake -B build-fuzz -DEZCLI_BUILD_FUZZ=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-fuzz
./build-fuzz/fuzz/fuzz_ez_cli fuzz/corpus/ -runs=500000
```

## Running tests

Tests use [doctest](https://github.com/doctest/doctest) (vendored single header; no download needed).

```sh
# Linux / macOS
ctest --test-dir build

# Windows
ctest --test-dir build -C Debug
```

For verbose output:

```sh
# Linux / macOS
./build/tests/test_ez_cli

# Windows
.\build\tests\Debug\test_ez_cli.exe
```

## Benchmarks

Benchmarks compare `cli_parse()` against hand-coded argument parsing — a minimal
`for` loop using direct `strcmp`/pointer comparisons with no config objects. This
establishes the overhead ratio of the abstraction layer.

Uses [nanobench](https://github.com/martinus/nanobench) v4.3.11, downloaded automatically
by CMake at configure time (requires internet access and git).

### Build

```sh
# Linux / macOS
cmake -B build -DEZCLI_BUILD_BENCHMARKS=ON
cmake --build build --config Release

# Windows
cmake -B build -G "Visual Studio 17 2022" -DEZCLI_BUILD_BENCHMARKS=ON
cmake --build build --config Release
```

### Run

```sh
# Linux / macOS
./build/benchmarks/bench_ez_cli

# Windows
.\build\benchmarks\Release\bench_ez_cli.exe
```

### Benchmark scenarios

| # | Scenario | argv | Notes |
|---|----------|------|-------|
| 1 | Short flags, separate | `-v -q -d` | 3 tokens, 3 defined flags |
| 2 | Short flags, clustered | `-vqd` | 1 token, 3 defined flags |
| 3 | Mixed | `-v --output=out.txt in.txt` | flag + long option (`=` form) + positional |
| 4 | Meta-option | `--help` | Generates full formatted help string; no hand-coded baseline |

### Indicative results (MSVC, Release, Windows 10)

| Scenario | ezcli | hand-coded | ratio |
|---|---|---|---|
| Short flags `-v -q -d` | ~95 ns | ~11 ns | ~9× |
| Clustered `-vqd` | ~54 ns | ~7 ns | ~8× |
| Mixed | ~124 ns | ~11 ns | ~11× |
| `--help` (help string) | ~3 000 ns | — | — |

**Key observations:**

- Normal parses cost **~55–125 ns** — well under 1 µs regardless of input shape.
- The abstraction overhead is **8–11×** compared to bare pointer/`strcmp` code,
  coming from `std::vector` config lookups and per-call result-object clearing.
- `--help` costs **~3 µs** due to the formatted help string being rebuilt on every
  call — entirely acceptable for a once-per-invocation code path.
- All of this is startup-time cost. `cli_parse` is called once per program run;
  even the slowest scenario adds under a microsecond to program startup.

> **Note:** some ezcli benchmark rows show nanobench "Unstable" markers because
> `cli_parse` performs a `std::string` heap allocation on every call (program-name
> extraction needed for potential meta-option output). The absolute values and ratios
> are consistent across runs despite the instability flag.

## Repository layout

```
ezcli/
  CMakeLists.txt
  ez_cli.h                public API + error codes
  ez_cli.cpp              implementation
  ez/                     ez standards submodule (EZ.md lives here)
  tests/
    CMakeLists.txt
    doctest.h             vendored single-header test framework
    test_ez_cli.cpp       unit + integration tests
  benchmarks/
    CMakeLists.txt
    bench_ez_cli.cpp      ezcli vs hand-coded argument parsing benchmark suite
  fuzz/
    CMakeLists.txt
    fuzz_ez_cli.cpp       libFuzzer target (Clang + WSL2/Linux only)
    corpus/               seed inputs derived from the test suite
```
