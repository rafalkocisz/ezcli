#pragma once

#include <string>
#include <string_view>
#include <vector>

// --- Return / error codes ---

#define EZ_CLI_OK                0   // successful parse
#define EZ_CLI_HELP_REQUESTED    1   // --help detected; message filled
#define EZ_CLI_USAGE_REQUESTED   2   // --usage detected; message filled
#define EZ_CLI_VERSION_REQUESTED 3   // --version detected; message filled
#define EZ_CLI_NO_MATCH          4   // unrecognised flag or option

#define EZ_CLI_ERR_SYNTAX       -1   // generic catch-all (reserved; not returned by current code)
#define EZ_CLI_ERR_CONFIG_NAME  -2   // invalid or duplicate flag/option/positional name
#define EZ_CLI_ERR_MISSING_VAL  -3   // option requires a value but none was provided
#define EZ_CLI_ERR_EMPTY_VAL    -4   // --option= with empty value after '='

namespace ez {

class CLIConfig;
class CLIFlags;
class CLIOptions;
class CLIArgs;

// argc, argv  — passed directly from main()
// config      — defines the expected CLI interface
// flags       — if non-null: filled with parsed flags on success; always cleared on entry
// options     — if non-null: filled with parsed options on success; always cleared on entry
// args        — if non-null: filled with parsed positional args on success; always cleared on entry
// message     — if non-null: filled with error description on failure, or pre-formatted
//               help/usage/version string when a meta-option is requested; always cleared on entry
//
// Returns:  0  — successful parse (EZ_CLI_OK)
//          >0  — meta-option requested (EZ_CLI_*_REQUESTED) or no match (EZ_CLI_NO_MATCH)
//          <0  — error (EZ_CLI_ERR_*)
int cli_parse(int argc, const char* const* argv,
              const CLIConfig& config,
              CLIFlags*    flags   = nullptr,
              CLIOptions*  options = nullptr,
              CLIArgs*     args    = nullptr,
              std::string* message = nullptr);

// ---------------------------------------------------------------------------

class CLIConfig {
public:
    // long_name: e.g. "verbose" — nullptr to omit; min 2 chars, lowercase alphanumeric + hyphens
    // short_name: e.g. 'v' — '\0' to omit; one alphanumeric character
    // At least one of long_name / short_name must be provided.
    // Returns EZ_CLI_OK or EZ_CLI_ERR_CONFIG_NAME — caller must assert.
    int add_flag(const char* long_name, char short_name, const char* description);
    int add_option(const char* long_name, char short_name, const char* description);

    // add_positional_list() must be called last (after all add_positional() calls)
    // and at most once — asserted.
    int add_positional(const char* name, const char* description);
    int add_positional_list(const char* name, const char* description);

    // Optional metadata for help/usage/version output.
    // If set_program_name() is not called, argv[0] basename is used.
    void set_version(const char* version);
    void set_program_name(const char* name);

private:
    friend int cli_parse(int, const char* const*, const CLIConfig&,
                         CLIFlags*, CLIOptions*, CLIArgs*, std::string*);

    struct Def {
        std::string long_name;   // empty if not provided
        char        short_name;  // '\0' if not provided
        std::string description;
    };

    struct PositionalDef {
        std::string name;
        std::string description;
        bool        is_list;
    };

    std::vector<Def>           flags_;
    std::vector<Def>           options_;
    std::vector<PositionalDef> positionals_;
    std::string                version_;       // empty if not set
    std::string                program_name_;  // empty if not set; falls back to argv[0] basename
};

// ---------------------------------------------------------------------------

class CLIFlags {
public:
    // name: long name (e.g. "verbose") or short name as single char (e.g. "v")
    bool is_set(const char* name) const;

private:
    friend int cli_parse(int, const char* const*, const CLIConfig&,
                         CLIFlags*, CLIOptions*, CLIArgs*, std::string*);

    struct SetFlag {
        std::string long_name;   // empty if no long name
        char        short_name;  // '\0' if no short name
    };

    std::vector<SetFlag> flags_;
};

// ---------------------------------------------------------------------------

class CLIOptions {
public:
    // name: long name or short name as single char
    bool             is_set(const char* name) const;
    std::string_view get(const char* name) const;  // empty string_view if not provided

private:
    friend int cli_parse(int, const char* const*, const CLIConfig&,
                         CLIFlags*, CLIOptions*, CLIArgs*, std::string*);

    struct SetOption {
        std::string      long_name;   // empty if no long name
        char             short_name;  // '\0' if no short name
        std::string_view value;       // view into argv
    };

    std::vector<SetOption> options_;
};

// ---------------------------------------------------------------------------

class CLIArgs {
public:
    std::string_view              get(const char* name) const;       // named positional
    std::vector<std::string_view> get_list(const char* name) const;  // variadic positional

private:
    friend int cli_parse(int, const char* const*, const CLIConfig&,
                         CLIFlags*, CLIOptions*, CLIArgs*, std::string*);

    struct SetArg {
        std::string      name;
        std::string_view value;  // view into argv
    };

    std::vector<SetArg>           args_;
    std::string                   list_name_;    // name of the variadic positional, empty if none
    std::vector<std::string_view> list_values_;  // views into argv
};

} // namespace ez
