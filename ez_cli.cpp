#include "ez_cli.h"
#include <cctype>
#include <cstring>

namespace {

bool is_valid_long_name(const char* name)
{
    if (!name) return false;
    size_t len = std::strlen(name);
    if (len < 2) return false;
    if (name[0] == '-' || name[len - 1] == '-') return false;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = name[i];
        if (!std::islower(c) && !std::isdigit(c) && c != '-') return false;
    }
    return true;
}

bool is_valid_short_name(char c)
{
    return std::isalnum((unsigned char)c) != 0;
}

bool is_valid_positional_name(const char* name)
{
    if (!name || name[0] == '\0') return false;
    for (const char* p = name; *p; ++p) {
        unsigned char c = *p;
        if (!std::islower(c) && !std::isdigit(c) && c != '-' && c != '_') return false;
    }
    return true;
}

} // namespace

namespace ez {

// ---------------------------------------------------------------------------
// CLIConfig

int CLIConfig::add_flag(const char* long_name, char short_name, const char* description)
{
    bool has_long  = (long_name != nullptr);
    bool has_short = (short_name != '\0');

    if (!has_long && !has_short)                        return EZ_CLI_ERR_CONFIG_NAME;
    if (has_long  && !is_valid_long_name(long_name))    return EZ_CLI_ERR_CONFIG_NAME;
    if (has_short && !is_valid_short_name(short_name))  return EZ_CLI_ERR_CONFIG_NAME;

    for (const auto& d : flags_) {
        if (has_long  && !d.long_name.empty() && d.long_name  == long_name)   return EZ_CLI_ERR_CONFIG_NAME;
        if (has_short && d.short_name != '\0' && d.short_name == short_name)  return EZ_CLI_ERR_CONFIG_NAME;
    }
    for (const auto& d : options_) {
        if (has_long  && !d.long_name.empty() && d.long_name  == long_name)   return EZ_CLI_ERR_CONFIG_NAME;
        if (has_short && d.short_name != '\0' && d.short_name == short_name)  return EZ_CLI_ERR_CONFIG_NAME;
    }

    flags_.push_back({has_long ? long_name : "", short_name, description ? description : ""});
    return EZ_CLI_OK;
}

int CLIConfig::add_option(const char* long_name, char short_name, const char* description)
{
    bool has_long  = (long_name != nullptr);
    bool has_short = (short_name != '\0');

    if (!has_long && !has_short)                        return EZ_CLI_ERR_CONFIG_NAME;
    if (has_long  && !is_valid_long_name(long_name))    return EZ_CLI_ERR_CONFIG_NAME;
    if (has_short && !is_valid_short_name(short_name))  return EZ_CLI_ERR_CONFIG_NAME;

    for (const auto& d : flags_) {
        if (has_long  && !d.long_name.empty() && d.long_name  == long_name)   return EZ_CLI_ERR_CONFIG_NAME;
        if (has_short && d.short_name != '\0' && d.short_name == short_name)  return EZ_CLI_ERR_CONFIG_NAME;
    }
    for (const auto& d : options_) {
        if (has_long  && !d.long_name.empty() && d.long_name  == long_name)   return EZ_CLI_ERR_CONFIG_NAME;
        if (has_short && d.short_name != '\0' && d.short_name == short_name)  return EZ_CLI_ERR_CONFIG_NAME;
    }

    options_.push_back({has_long ? long_name : "", short_name, description ? description : ""});
    return EZ_CLI_OK;
}

int CLIConfig::add_positional(const char* name, const char* description)
{
    if (!is_valid_positional_name(name)) return EZ_CLI_ERR_CONFIG_NAME;

    for (const auto& p : positionals_) {
        if (p.is_list)          return EZ_CLI_ERR_CONFIG_NAME;  // list already added — must be last
        if (p.name == name)     return EZ_CLI_ERR_CONFIG_NAME;  // duplicate name
    }

    positionals_.push_back({name, description ? description : "", false});
    return EZ_CLI_OK;
}

int CLIConfig::add_positional_list(const char* name, const char* description)
{
    if (!is_valid_positional_name(name)) return EZ_CLI_ERR_CONFIG_NAME;

    for (const auto& p : positionals_) {
        if (p.is_list)      return EZ_CLI_ERR_CONFIG_NAME;  // already has a list
        if (p.name == name) return EZ_CLI_ERR_CONFIG_NAME;  // duplicate name
    }

    positionals_.push_back({name, description ? description : "", true});
    return EZ_CLI_OK;
}

void CLIConfig::set_version(const char* version)
{
    version_ = version ? version : "";
}

void CLIConfig::set_program_name(const char* name)
{
    program_name_ = name ? name : "";
}

void CLIConfig::set_usage(const char* usage)
{
    usage_ = usage ? usage : "";
}

// ---------------------------------------------------------------------------
// CLIFlags

bool CLIFlags::is_set(const char* name) const
{
    if (!name) return false;
    bool is_short = (name[0] != '\0' && name[1] == '\0');
    for (const auto& f : flags_) {
        if (is_short) {
            if (f.short_name == name[0]) return true;
        } else {
            if (!f.long_name.empty() && f.long_name == name) return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// CLIOptions

bool CLIOptions::is_set(const char* name) const
{
    if (!name) return false;
    bool is_short = (name[0] != '\0' && name[1] == '\0');
    for (const auto& o : options_) {
        if (is_short) {
            if (o.short_name == name[0]) return true;
        } else {
            if (!o.long_name.empty() && o.long_name == name) return true;
        }
    }
    return false;
}

std::string_view CLIOptions::get(const char* name) const
{
    if (!name) return {};
    bool is_short = (name[0] != '\0' && name[1] == '\0');
    for (const auto& o : options_) {
        if (is_short) {
            if (o.short_name == name[0]) return o.value;
        } else {
            if (!o.long_name.empty() && o.long_name == name) return o.value;
        }
    }
    return {};
}

// ---------------------------------------------------------------------------
// CLIArgs

std::string_view CLIArgs::get(const char* name) const
{
    if (!name) return {};
    for (const auto& a : args_) {
        if (a.name == name) return a.value;
    }
    return {};
}

std::vector<std::string_view> CLIArgs::get_list(const char* name) const
{
    if (name && list_name_ == name) return list_values_;
    return {};
}

// ---------------------------------------------------------------------------
// cli_parse

int cli_parse(int argc, const char* const* argv,
              const CLIConfig& config,
              CLIFlags*    flags,
              CLIOptions*  options,
              CLIArgs*     args,
              std::string* message)
{
    // Always clear on entry regardless of outcome
    if (flags)   flags->flags_.clear();
    if (options) options->options_.clear();
    if (args) {
        args->args_.clear();
        args->list_name_.clear();
        args->list_values_.clear();
    }
    if (message) message->clear();

    int i = 1;  // skip argv[0] (program name)

    while (i < argc) {
        const char* token = argv[i];

        if (token[0] == '-' && token[1] != '-' && token[1] != '\0') {
            // Short flag / option cluster: -x, -xyz
            for (const char* p = token + 1; *p != '\0'; ++p) {
                char c = *p;

                // Look up as a flag
                bool found = false;
                for (const auto& d : config.flags_) {
                    if (d.short_name == c) {
                        found = true;
                        if (flags) flags->flags_.push_back({d.long_name, d.short_name});
                        break;
                    }
                }
                if (found) continue;

                // Phase 4.3: option handling inserted here

                if (message) { *message = "unknown option: -"; *message += c; }
                return EZ_CLI_NO_MATCH;
            }
        }
        // Phase 4.4/4.5: long options (--)
        // Phase 4.6: positionals
        // Phase 4.7: meta-options

        ++i;
    }

    return EZ_CLI_OK;
}

} // namespace ez
