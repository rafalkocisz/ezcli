#include "ez_cli.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// Fixed config that exercises all parser paths: short flags, long flags,
// short value options, long value options (space + = forms), positionals,
// positional list, meta-options, '--' separator, and unknown-token errors.
static ez::CLIConfig make_config()
{
    ez::CLIConfig c;
    c.set_program_name("fuzz");
    c.set_version("0");
    c.add_flag("verbose", 'v', "verbose");
    c.add_flag("dry-run", '\0', "dry run");
    c.add_option("output", 'o', "output file");
    c.add_option("count",  '\0', "count");
    c.add_positional("input", "input file");
    c.add_positional_list("extras", "extra files");
    return c;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Append a null byte so the last token is always null-terminated,
    // then split on null bytes to build argv.
    std::string buf(reinterpret_cast<const char*>(data), size);
    buf += '\0';

    std::vector<const char*> argv = {"fuzz"};
    const char* p   = buf.data();
    const char* end = buf.data() + buf.size() - 1;  // exclude the appended '\0'

    while (p < end) {
        if (*p != '\0') {
            argv.push_back(p);
            while (p < end && *p != '\0') ++p;
        }
        ++p;
    }

    static const ez::CLIConfig config = make_config();

    ez::CLIFlags   flags;
    ez::CLIOptions options;
    ez::CLIArgs    args;
    std::string    message;

    ez::cli_parse(static_cast<int>(argv.size()), argv.data(),
                  config, &flags, &options, &args, &message);
    return 0;
}
