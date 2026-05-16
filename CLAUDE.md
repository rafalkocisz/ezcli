# ezcli — Project Constitution

> Non-domain-specific style rules (Orthodox C++, code formatting, testing, build options,
> benchmarking, sanitizers, fuzzing, naming, repository layout) live in **[ez/EZ.md](ez/EZ.md)**
> and apply to this project in full. This file contains only ezcli-specific decisions.

## Purpose
A minimal, dependency-free C++ command line argument parsing library. Handles definition
of flags, value options, and positional arguments via `ez::CLIConfig`; parses `argc`/`argv`
via `ez::cli_parse()`; and returns results through `ez::CLIFlags`, `ez::CLIOptions`, and
`ez::CLIArgs`. Never prints — all output is returned as strings or codes. No subcommands,
no optional option arguments, no Unicode.

## Public API

```cpp
namespace ez {

class CLIConfig {
public:
    // long_name: long option name (e.g. "verbose"), or nullptr to omit
    // short_name: short option character (e.g. 'v'), or '\0' to omit
    // description: shown in help/usage output
    // Returns EZ_CLI_OK or EZ_CLI_ERR_CONFIG_* — caller must assert
    int add_flag(const char* long_name, char short_name, const char* description);
    int add_option(const char* long_name, char short_name, const char* description);

    // name: positional argument name, shown in help/usage output
    // add_positional_list() must be called last and only once — asserted
    int add_positional(const char* name, const char* description);
    int add_positional_list(const char* name, const char* description);
};

class CLIFlags {
public:
    bool is_set(const char* name) const;  // query by long or short name
};

class CLIOptions {
public:
    // returns empty string_view if option was not provided
    std::string_view get(const char* name) const;
    bool is_set(const char* name) const;
};

class CLIArgs {
public:
    std::string_view get(const char* name) const;        // named positional
    std::vector<std::string_view> get_list(const char* name) const; // variadic positional
};

// argc, argv  — passed directly from main()
// config      — defines the expected CLI interface
// flags       — if non-null: filled with parsed flags on success
// options     — if non-null: filled with parsed options on success
// args        — if non-null: filled with parsed positional arguments on success
// message     — if non-null: filled with error message on failure, or pre-formatted
//               help/usage/version string when a meta-option is requested;
//               cleared on every call
//
// Returns:  0  — successful parse (use EZ_CLI_OK)
//          >0  — meta-option requested or no match (use EZ_CLI_HELP_REQUESTED,
//                EZ_CLI_USAGE_REQUESTED, EZ_CLI_VERSION_REQUESTED, EZ_CLI_NO_MATCH)
//          <0  — error (use EZ_CLI_ERR_* codes)
int cli_parse(int argc, const char* const* argv,
              const CLIConfig& config,
              CLIFlags* flags = nullptr,
              CLIOptions* options = nullptr,
              CLIArgs* args = nullptr,
              std::string* message = nullptr);

} // namespace ez
```

## Supported CLI Conventions

| Convention | Detail |
|------------|--------|
| Short flags | `-a`, `-v`, `-Z` — single hyphen + one alphanumeric character |
| Short flag clustering | `-abc` equivalent to `-a -b -c` |
| Short value options | `-o file` or `-ofile` (packed, no space) |
| Short cluster + value | Value option must be last: `-abco file` or `-abcofile` |
| Long flags | `--verbose`, `--ignore-backups` — two hyphens + lowercase alphanumeric words separated by hyphens, minimum 2 characters |
| Long value options | `--output file` or `--output=file` |
| Short + long combined | `-v` / `--verbose` — both forms defined together, both equivalent |
| `--` separator | Everything after `--` treated as positional arguments |
| Meta-options | `--help`, `--usage`, `--version` — built-in, long form only, no config needed |

**Not supported:** subcommands, optional option arguments, prefix/abbreviation matching,
`--man`, Unicode.

## Parsing Semantics
- Flags and options may appear in any order before positional arguments.
- `--` explicitly ends option parsing; all remaining argv entries are positionals.
- Without `--`, a token not starting with `-` ends option parsing and begins positionals.
- Short option value starting with `-`: consumed as the value; use `--` to disambiguate.
- `--option=` with empty value after `=` is an error.
- `message` is always cleared on entry regardless of outcome.
- `CLIFlags`, `CLIOptions`, and `CLIArgs` are cleared on every `cli_parse()` call.
- String views in results are valid as long as `argv` is alive.
- Meta-options (`--help`, `--usage`, `--version`) take priority; when detected,
  `message` is filled with the pre-formatted string and a positive code is returned.

## Return / Error codes

```cpp
#define EZ_CLI_OK                0   // successful parse
#define EZ_CLI_HELP_REQUESTED    1   // --help detected; message filled
#define EZ_CLI_USAGE_REQUESTED   2   // --usage detected; message filled
#define EZ_CLI_VERSION_REQUESTED 3   // --version detected; message filled
#define EZ_CLI_NO_MATCH          4   // unrecognised flag or option
#define EZ_CLI_ERR_SYNTAX       -1   // generic catch-all (reserved)
#define EZ_CLI_ERR_CONFIG_NAME  -2   // invalid or duplicate flag/option/positional name
#define EZ_CLI_ERR_MISSING_VAL  -3   // option requires a value but none was provided
#define EZ_CLI_ERR_EMPTY_VAL    -4   // --option= with empty value after '='
```

Config errors (`EZ_CLI_ERR_CONFIG_*`) are programmer errors — assert on the return value
of `add_flag()`, `add_option()`, `add_positional()`, `add_positional_list()`.
Runtime errors (`EZ_CLI_NO_MATCH`, `EZ_CLI_ERR_MISSING_VAL`, etc.) are user input errors —
handle them by presenting `message` to the user.

## Coding Standards
See [EZ.md § Code Style](ez/EZ.md#code-style--orthodox-c). Domain-specific additions:
- `std::vector<std::string_view>` is used for variadic positional argument storage —
  acceptable per EZ.md since CLI parsing is a one-time startup operation.
- `std::string` is used for the error/message output parameter only.

## Naming Conventions
See [EZ.md § Naming Conventions](ez/EZ.md#naming-conventions) for the full table and
rationale. ezcli-specific instantiation:

| Element | Name |
|---------|------|
| Namespace | `ez` |
| Public function | `ez::cli_parse` |
| Public classes | `ez::CLIConfig`, `ez::CLIFlags`, `ez::CLIOptions`, `ez::CLIArgs` |
| Return-code macros | `EZ_CLI_OK`, `EZ_CLI_NO_MATCH`, `EZ_CLI_ERR_*` |
| Private member variables | `snake_case_` (trailing underscore) |
| White-box testing flag | `EZ_CLI_TESTING` |
| CMake options | `EZCLI_BUILD_TESTS`, `EZCLI_BUILD_BENCHMARKS`, `EZCLI_SANITIZE`, `EZCLI_BUILD_FUZZ` |

## Architecture
- Single-pass parser over `argv`. No intermediate representation.
- `CLIConfig` stores flag/option/positional definitions in `std::vector` (one-time
  startup allocation, not performance-sensitive).
- `CLIFlags`, `CLIOptions`, `CLIArgs` store results as `std::string_view` into `argv`.
- Source files: `ez_cli.h`, `ez_cli.cpp`.
- Unit tests: `tests/test_ez_cli.cpp` using the **doctest** framework.
- Build system: CMake ≥ 3.20.

## Repository Layout
Follows the canonical ez layout from [EZ.md § Repository Layout](ez/EZ.md#repository-layout)
in full, including `benchmarks/` and `fuzz/`, plus the `ez/` submodule.

## Commands

```sh
# Configure + build (normal)
cmake -B build && cmake --build build

# Run tests
ctest --test-dir build

# Sanitizer build (Clang, WSL2/Linux)
cmake -B build-san -DEZCLI_SANITIZE=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-san && ./build-san/tests/test_ez_cli

# Fuzz build (Clang, WSL2/Linux)
cmake -B build-fuzz -DEZCLI_BUILD_FUZZ=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-fuzz
./build-fuzz/fuzz/fuzz_ez_cli fuzz/corpus/ -runs=500000
```

## Out of Scope
- Subcommands.
- Optional option arguments.
- Prefix/abbreviation matching for long options.
- `--man` meta-option.
- Unicode / multibyte option names or values.
- Printing — the library never writes to stdout/stderr.
- Constraint enforcement (required options, mutually exclusive flags, etc.) — left to the caller.
