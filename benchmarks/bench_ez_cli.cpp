#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include "ez_cli.h"
#include <cstring>

// ---------------------------------------------------------------------------
// Hand-coded baselines — same logical work, no CLIConfig overhead
// ---------------------------------------------------------------------------

static void hand_short_flags(int argc, const char* const* argv,
                              bool& v, bool& q, bool& d)
{
    v = q = d = false;
    for (int i = 1; i < argc; ++i) {
        const char* t = argv[i];
        if (t[0] != '-' || t[1] == '-' || t[1] == '\0') continue;
        for (const char* p = t + 1; *p; ++p) {
            if      (*p == 'v') v = true;
            else if (*p == 'q') q = true;
            else if (*p == 'd') d = true;
        }
    }
}

static void hand_mixed(int argc, const char* const* argv,
                        bool& v, const char*& output, const char*& input)
{
    v = false; output = nullptr; input = nullptr;
    for (int i = 1; i < argc; ++i) {
        const char* t = argv[i];
        if (t[0] == '-') {
            if (t[1] == '-') {
                if (std::strncmp(t, "--output=", 9) == 0) output = t + 9;
                else if (std::strcmp(t, "--verbose") == 0) v = true;
            } else {
                for (const char* p = t + 1; *p; ++p)
                    if (*p == 'v') v = true;
            }
        } else {
            input = t;
        }
    }
}

// ---------------------------------------------------------------------------
// Benchmarks
// ---------------------------------------------------------------------------

int main()
{
    namespace nb = ankerl::nanobench;
    nb::Bench bench;
    bench.title("ezcli vs hand-coded").unit("parse")
         .warmup(200).minEpochIterations(10000);

    // -----------------------------------------------------------------------
    // Scenario 1: three short flags — separate tokens (-v -q -d)
    // -----------------------------------------------------------------------
    {
        ez::CLIConfig config;
        config.add_flag("verbose", 'v', nullptr);
        config.add_flag("quiet",   'q', nullptr);
        config.add_flag("dry-run", 'd', nullptr);
        const char* argv[] = {"prog", "-v", "-q", "-d"};
        ez::CLIFlags flags;

        bench.run("ezcli  short flags  -v -q -d", [&] {
            ez::cli_parse(4, argv, config, &flags);
            nb::doNotOptimizeAway(flags);
        });
        bool v, q, d;
        bench.run("hand   short flags  -v -q -d", [&] {
            hand_short_flags(4, argv, v, q, d);
            nb::doNotOptimizeAway(v);
        });
    }

    // -----------------------------------------------------------------------
    // Scenario 2: three short flags — clustered (-vqd)
    // -----------------------------------------------------------------------
    {
        ez::CLIConfig config;
        config.add_flag("verbose", 'v', nullptr);
        config.add_flag("quiet",   'q', nullptr);
        config.add_flag("dry-run", 'd', nullptr);
        const char* argv[] = {"prog", "-vqd"};
        ez::CLIFlags flags;

        bench.run("ezcli  clustered     -vqd", [&] {
            ez::cli_parse(2, argv, config, &flags);
            nb::doNotOptimizeAway(flags);
        });
        bool v, q, d;
        bench.run("hand   clustered     -vqd", [&] {
            hand_short_flags(2, argv, v, q, d);
            nb::doNotOptimizeAway(v);
        });
    }

    // -----------------------------------------------------------------------
    // Scenario 3: typical mixed — flag + long option (=) + positional
    // -----------------------------------------------------------------------
    {
        ez::CLIConfig config;
        config.add_flag("verbose", 'v', nullptr);
        config.add_option("output", 'o', nullptr);
        config.add_positional("input", nullptr);
        const char* argv[] = {"prog", "-v", "--output=out.txt", "in.txt"};
        ez::CLIFlags   flags;
        ez::CLIOptions options;
        ez::CLIArgs    args;

        bench.run("ezcli  mixed  -v --output=out.txt in.txt", [&] {
            ez::cli_parse(4, argv, config, &flags, &options, &args);
            nb::doNotOptimizeAway(flags);
            nb::doNotOptimizeAway(options);
            nb::doNotOptimizeAway(args);
        });
        bool v; const char* output; const char* input;
        bench.run("hand   mixed  -v --output=out.txt in.txt", [&] {
            hand_mixed(4, argv, v, output, input);
            nb::doNotOptimizeAway(v);
            nb::doNotOptimizeAway(input);
        });
    }

    // -----------------------------------------------------------------------
    // Scenario 4: --help  (meta-option — short-circuits, generates help string)
    // No hand-coded baseline: formatted string generation has no equivalent.
    // -----------------------------------------------------------------------
    {
        ez::CLIConfig config;
        config.set_program_name("prog");
        config.set_version("1.0.0");
        config.add_flag("verbose", 'v', "Enable verbose output");
        config.add_flag("dry-run", '\0', "Simulate without writing");
        config.add_option("output", 'o', "Output file path");
        config.add_positional("input", "Input file to process");
        const char* argv[] = {"prog", "--help"};
        std::string msg;

        bench.run("ezcli  --help (generates help string)", [&] {
            ez::cli_parse(2, argv, config, nullptr, nullptr, nullptr, &msg);
            nb::doNotOptimizeAway(msg);
        });
    }
}
