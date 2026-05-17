#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "ez_cli.h"
#include <cassert>

// ---------------------------------------------------------------------------
// cli_parse — 4.1: output clearing + no-arguments case
// ---------------------------------------------------------------------------

TEST_SUITE("cli_parse — 4.1 output clearing and no-args")
{
    TEST_CASE("no arguments — returns OK")
    {
        ez::CLIConfig config;
        const char* argv[] = {"prog"};
        CHECK(ez::cli_parse(1, argv, config) == EZ_CLI_OK);
    }

    TEST_CASE("all output pointers null — no crash")
    {
        ez::CLIConfig config;
        const char* argv[] = {"prog"};
        CHECK(ez::cli_parse(1, argv, config, nullptr, nullptr, nullptr, nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("message is cleared on entry")
    {
        ez::CLIConfig config;
        const char* argv[] = {"prog"};
        std::string msg = "leftover";
        CHECK(ez::cli_parse(1, argv, config, nullptr, nullptr, nullptr, &msg) == EZ_CLI_OK);
        CHECK(msg.empty());
    }

    TEST_CASE("empty config with no args — returns OK")
    {
        ez::CLIConfig config;
        const char* argv[] = {"prog"};
        ez::CLIFlags   flags;
        ez::CLIOptions options;
        ez::CLIArgs    args;
        std::string    msg;
        CHECK(ez::cli_parse(1, argv, config, &flags, &options, &args, &msg) == EZ_CLI_OK);
        CHECK(msg.empty());
    }
}

// ---------------------------------------------------------------------------
// cli_parse — 4.2: short flags + clustering
// ---------------------------------------------------------------------------

TEST_SUITE("cli_parse — 4.2 short flags")
{
    TEST_CASE("single flag — is_set by short name")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-v"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
    }

    TEST_CASE("single flag — is_set by long name")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-v"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("verbose"));
    }

    TEST_CASE("short-name-only flag")
    {
        ez::CLIConfig config;
        int r = config.add_flag(nullptr, 'x', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-x"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("x"));
    }

    TEST_CASE("multiple separate flags")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("quiet",   'q', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-v", "-q"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(3, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
        CHECK(flags.is_set("q"));
    }

    TEST_CASE("clustered flags")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("quiet",   'q', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("dry-run", 'd', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-vqd"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
        CHECK(flags.is_set("q"));
        CHECK(flags.is_set("d"));
    }

    TEST_CASE("unset flag returns false")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(1, argv, config, &flags) == EZ_CLI_OK);
        CHECK(!flags.is_set("v"));
        CHECK(!flags.is_set("verbose"));
    }

    TEST_CASE("duplicate flag in argv — no error")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-v", "-v"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(3, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
    }

    TEST_CASE("flags cleared between calls")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("quiet",   'q', nullptr); assert(r == EZ_CLI_OK);
        const char* argv1[] = {"prog", "-v"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv1, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
        const char* argv2[] = {"prog", "-q"};
        CHECK(ez::cli_parse(2, argv2, config, &flags) == EZ_CLI_OK);
        CHECK(!flags.is_set("v"));
        CHECK(flags.is_set("q"));
    }

    TEST_CASE("null flags pointer — no crash")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-v"};
        CHECK(ez::cli_parse(2, argv, config, nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("unknown short flag — NO_MATCH with message")
    {
        ez::CLIConfig config;
        const char* argv[] = {"prog", "-x"};
        std::string msg;
        CHECK(ez::cli_parse(2, argv, config, nullptr, nullptr, nullptr, &msg) == EZ_CLI_NO_MATCH);
        CHECK(!msg.empty());
    }

    TEST_CASE("unknown flag in cluster — NO_MATCH")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-vx"};
        std::string msg;
        CHECK(ez::cli_parse(2, argv, config, nullptr, nullptr, nullptr, &msg) == EZ_CLI_NO_MATCH);
        CHECK(!msg.empty());
    }
}

// ---------------------------------------------------------------------------
// CLIConfig::add_flag
// ---------------------------------------------------------------------------

TEST_SUITE("CLIConfig::add_flag")
{
    TEST_CASE("both names — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose", 'v', "Enable verbose output") == EZ_CLI_OK);
    }

    TEST_CASE("long name only — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose", '\0', "description") == EZ_CLI_OK);
    }

    TEST_CASE("short name only — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag(nullptr, 'v', "description") == EZ_CLI_OK);
    }

    TEST_CASE("uppercase short name — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose", 'V', "description") == EZ_CLI_OK);
    }

    TEST_CASE("digit short name — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("level1", '1', "description") == EZ_CLI_OK);
    }

    TEST_CASE("long name with digits — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("mp3gain", '\0', "description") == EZ_CLI_OK);
    }

    TEST_CASE("null description — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose", 'v', nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("multiple flags — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose",  'v', nullptr) == EZ_CLI_OK);
        CHECK(c.add_flag("dry-run",  '\0', nullptr) == EZ_CLI_OK);
        CHECK(c.add_flag(nullptr,    'q', nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("both names absent")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag(nullptr, '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name too short — one character")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("v", '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name too short — empty string")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("", '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name with uppercase letter")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("Verbose", '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name with underscore")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("dry_run", '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name with space")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("dry run", '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name starts with hyphen")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("-verbose", '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name ends with hyphen")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose-", '\0', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("short name is hyphen")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag(nullptr, '-', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("short name is space")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag(nullptr, ' ', "description") == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("duplicate long name in flags")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose", '\0', nullptr) == EZ_CLI_OK);
        CHECK(c.add_flag("verbose", '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("duplicate short name in flags")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag(nullptr, 'v', nullptr) == EZ_CLI_OK);
        CHECK(c.add_flag(nullptr, 'v', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name already used by an option")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("output", 'o', nullptr) == EZ_CLI_OK);
        CHECK(c.add_flag("output",  '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("short name already used by an option")
    {
        ez::CLIConfig c;
        CHECK(c.add_option(nullptr, 'o', nullptr) == EZ_CLI_OK);
        CHECK(c.add_flag(nullptr,   'o', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }
}

// ---------------------------------------------------------------------------
// CLIConfig::add_option
// ---------------------------------------------------------------------------

TEST_SUITE("CLIConfig::add_option")
{
    TEST_CASE("both names — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("output", 'o', "Output file") == EZ_CLI_OK);
    }

    TEST_CASE("long name only — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("output", '\0', nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("short name only — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_option(nullptr, 'o', nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("multiple options — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("output", 'o', nullptr) == EZ_CLI_OK);
        CHECK(c.add_option("input",  'i', nullptr) == EZ_CLI_OK);
        CHECK(c.add_option("count",  '\0', nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("null description — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("output", 'o', nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("both names absent")
    {
        ez::CLIConfig c;
        CHECK(c.add_option(nullptr, '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name too short")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("o", '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name with uppercase")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("Output", '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name starts with hyphen")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("-output", '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name ends with hyphen")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("output-", '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("short name is not alphanumeric")
    {
        ez::CLIConfig c;
        CHECK(c.add_option(nullptr, '!', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("duplicate long name in options")
    {
        ez::CLIConfig c;
        CHECK(c.add_option("output", '\0', nullptr) == EZ_CLI_OK);
        CHECK(c.add_option("output", '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("duplicate short name in options")
    {
        ez::CLIConfig c;
        CHECK(c.add_option(nullptr, 'o', nullptr) == EZ_CLI_OK);
        CHECK(c.add_option(nullptr, 'o', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("long name already used by a flag")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag("verbose", '\0', nullptr) == EZ_CLI_OK);
        CHECK(c.add_option("verbose", '\0', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("short name already used by a flag")
    {
        ez::CLIConfig c;
        CHECK(c.add_flag(nullptr, 'v', nullptr) == EZ_CLI_OK);
        CHECK(c.add_option(nullptr, 'v', nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }
}

// ---------------------------------------------------------------------------
// CLIConfig::add_positional
// ---------------------------------------------------------------------------

TEST_SUITE("CLIConfig::add_positional")
{
    TEST_CASE("single positional — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("input", "Input file") == EZ_CLI_OK);
    }

    TEST_CASE("multiple positionals — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("input",  nullptr) == EZ_CLI_OK);
        CHECK(c.add_positional("output", nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("name with hyphen — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("input-file", nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("name with underscore — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("input_file", nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("name with digit — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("file2", nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("null name")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional(nullptr, nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("empty name")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("name with uppercase")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("Input", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("name with space")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("input file", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("duplicate positional name")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("input", nullptr) == EZ_CLI_OK);
        CHECK(c.add_positional("input", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("add after positional_list")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional_list("files", nullptr) == EZ_CLI_OK);
        CHECK(c.add_positional("extra", nullptr)      == EZ_CLI_ERR_CONFIG_NAME);
    }
}

// ---------------------------------------------------------------------------
// CLIConfig::add_positional_list
// ---------------------------------------------------------------------------

TEST_SUITE("CLIConfig::add_positional_list")
{
    TEST_CASE("alone — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional_list("files", "Input files") == EZ_CLI_OK);
    }

    TEST_CASE("name with underscore — valid")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional_list("input_files", nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("null name")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional_list(nullptr, nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("empty name")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional_list("", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("name with uppercase")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional_list("Files", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("duplicate name — same as a positional")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional("files",      nullptr) == EZ_CLI_OK);
        CHECK(c.add_positional_list("files", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }

    TEST_CASE("called twice")
    {
        ez::CLIConfig c;
        CHECK(c.add_positional_list("files",  nullptr) == EZ_CLI_OK);
        CHECK(c.add_positional_list("extras", nullptr) == EZ_CLI_ERR_CONFIG_NAME);
    }
}

// ---------------------------------------------------------------------------
// cli_parse — 4.3: short value options
// ---------------------------------------------------------------------------

TEST_SUITE("cli_parse — 4.3 short value options")
{
    TEST_CASE("space-separated: -o file")
    {
        ez::CLIConfig config;
        int r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-o", "file.txt"};
        ez::CLIOptions options;
        CHECK(ez::cli_parse(3, argv, config, nullptr, &options) == EZ_CLI_OK);
        CHECK(options.is_set("o"));
        CHECK(options.is_set("output"));
        CHECK(options.get("o")      == "file.txt");
        CHECK(options.get("output") == "file.txt");
    }

    TEST_CASE("packed: -ofile")
    {
        ez::CLIConfig config;
        int r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-ofile.txt"};
        ez::CLIOptions options;
        CHECK(ez::cli_parse(2, argv, config, nullptr, &options) == EZ_CLI_OK);
        CHECK(options.is_set("o"));
        CHECK(options.get("o") == "file.txt");
    }

    TEST_CASE("flags before option, space: -vqo file")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("quiet",   'q', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-vqo", "out.txt"};
        ez::CLIFlags   flags;
        ez::CLIOptions options;
        CHECK(ez::cli_parse(3, argv, config, &flags, &options) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
        CHECK(flags.is_set("q"));
        CHECK(options.is_set("o"));
        CHECK(options.get("o") == "out.txt");
    }

    TEST_CASE("flags before option, packed: -vqoout.txt")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("quiet",   'q', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-vqoout.txt"};
        ez::CLIFlags   flags;
        ez::CLIOptions options;
        CHECK(ez::cli_parse(2, argv, config, &flags, &options) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
        CHECK(flags.is_set("q"));
        CHECK(options.is_set("o"));
        CHECK(options.get("o") == "out.txt");
    }

    TEST_CASE("option not provided — is_set false, get empty")
    {
        ez::CLIConfig config;
        int r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog"};
        ez::CLIOptions options;
        CHECK(ez::cli_parse(1, argv, config, nullptr, &options) == EZ_CLI_OK);
        CHECK(!options.is_set("o"));
        CHECK(!options.is_set("output"));
        CHECK(options.get("o").empty());
        CHECK(options.get("output").empty());
    }

    TEST_CASE("missing value at end of argv — MISSING_VAL with message")
    {
        ez::CLIConfig config;
        int r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-o"};
        std::string msg;
        CHECK(ez::cli_parse(2, argv, config, nullptr, nullptr, nullptr, &msg) == EZ_CLI_ERR_MISSING_VAL);
        CHECK(!msg.empty());
    }

    TEST_CASE("missing value in cluster at end of argv — MISSING_VAL")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-vo"};
        std::string msg;
        CHECK(ez::cli_parse(2, argv, config, nullptr, nullptr, nullptr, &msg) == EZ_CLI_ERR_MISSING_VAL);
        CHECK(!msg.empty());
    }

    TEST_CASE("options cleared between calls")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_option("input",  'i', nullptr); assert(r == EZ_CLI_OK);
        const char* argv1[] = {"prog", "-o", "out.txt"};
        ez::CLIOptions options;
        CHECK(ez::cli_parse(3, argv1, config, nullptr, &options) == EZ_CLI_OK);
        CHECK(options.is_set("o"));
        const char* argv2[] = {"prog", "-i", "in.txt"};
        CHECK(ez::cli_parse(3, argv2, config, nullptr, &options) == EZ_CLI_OK);
        CHECK(!options.is_set("o"));
        CHECK(options.is_set("i"));
        CHECK(options.get("i") == "in.txt");
    }

    TEST_CASE("null options pointer — no crash")
    {
        ez::CLIConfig config;
        int r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-o", "file.txt"};
        CHECK(ez::cli_parse(3, argv, config, nullptr, nullptr) == EZ_CLI_OK);
    }

    TEST_CASE("short-name-only option")
    {
        ez::CLIConfig config;
        int r = config.add_option(nullptr, 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-o", "file.txt"};
        ez::CLIOptions options;
        CHECK(ez::cli_parse(3, argv, config, nullptr, &options) == EZ_CLI_OK);
        CHECK(options.is_set("o"));
        CHECK(options.get("o") == "file.txt");
    }

    TEST_CASE("value starting with hyphen — consumed as value")
    {
        ez::CLIConfig config;
        int r = config.add_option("output", 'o', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "-o", "-myfile"};
        ez::CLIOptions options;
        CHECK(ez::cli_parse(3, argv, config, nullptr, &options) == EZ_CLI_OK);
        CHECK(options.get("o") == "-myfile");
    }
}

// ---------------------------------------------------------------------------
// cli_parse — 4.4: long flags
// ---------------------------------------------------------------------------

TEST_SUITE("cli_parse — 4.4 long flags")
{
    TEST_CASE("long flag — is_set by long name")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "--verbose"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("verbose"));
    }

    TEST_CASE("long flag — is_set by short name")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "--verbose"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("v"));
    }

    TEST_CASE("long-name-only flag")
    {
        ez::CLIConfig config;
        int r = config.add_flag("dry-run", '\0', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "--dry-run"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(2, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("dry-run"));
    }

    TEST_CASE("multiple long flags")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("dry-run", '\0', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "--verbose", "--dry-run"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(3, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("verbose"));
        CHECK(flags.is_set("dry-run"));
    }

    TEST_CASE("long and short flags mixed")
    {
        ez::CLIConfig config;
        int r;
        r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        r = config.add_flag("quiet",   'q', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "--verbose", "-q"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(3, argv, config, &flags) == EZ_CLI_OK);
        CHECK(flags.is_set("verbose"));
        CHECK(flags.is_set("q"));
    }

    TEST_CASE("unset long flag returns false")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog"};
        ez::CLIFlags flags;
        CHECK(ez::cli_parse(1, argv, config, &flags) == EZ_CLI_OK);
        CHECK(!flags.is_set("verbose"));
    }

    TEST_CASE("unknown long flag — NO_MATCH with message")
    {
        ez::CLIConfig config;
        const char* argv[] = {"prog", "--unknown"};
        std::string msg;
        CHECK(ez::cli_parse(2, argv, config, nullptr, nullptr, nullptr, &msg) == EZ_CLI_NO_MATCH);
        CHECK(!msg.empty());
    }

    TEST_CASE("null flags pointer — no crash")
    {
        ez::CLIConfig config;
        int r = config.add_flag("verbose", 'v', nullptr); assert(r == EZ_CLI_OK);
        const char* argv[] = {"prog", "--verbose"};
        CHECK(ez::cli_parse(2, argv, config, nullptr) == EZ_CLI_OK);
    }
}
