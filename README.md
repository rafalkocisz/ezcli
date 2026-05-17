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
- No heap allocation in the option definitions (fixed-size arrays); `std::vector` used
  only for variadic positional results

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

| Sanitizer | MSVC (VS 2022) | GCC / Clang |
|---|---|---|
| ASan (Address Sanitizer) | `/fsanitize=address` | `-fsanitize=address` |
| UBSan (Undefined Behaviour) | not supported | `-fsanitize=undefined` |

### Build

```sh
# Linux / macOS — ASan + UBSan
cmake -B build-san -DEZCLI_SANITIZE=ON
cmake --build build-san

# Windows (ASan only)
cmake -B build-san -G "Visual Studio 17 2022" -DEZCLI_SANITIZE=ON
cmake --build build-san --config RelWithDebInfo
```

### Run

```sh
# Linux / macOS
./build-san/tests/test_ez_cli

# Windows
$asanDir = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x64"
$env:PATH = "$asanDir;$env:PATH"
.\build-san\tests\RelWithDebInfo\test_ez_cli.exe
```

For UBSan on Windows, use WSL2 with Clang:

```sh
cmake -B build-san -DEZCLI_SANITIZE=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-san
./build-san/tests/test_ez_cli
```

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
