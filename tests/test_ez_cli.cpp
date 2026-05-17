#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "ez_cli.h"

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
