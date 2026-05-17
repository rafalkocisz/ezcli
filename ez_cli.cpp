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

    // --- Program name (used for meta-option output) ---
    std::string prog;
    if (!config.program_name_.empty()) {
        prog = config.program_name_;
    } else {
        const char* p = argv[0];
        const char* start = argv[0];
        for (; *p; ++p)
            if (*p == '/' || *p == '\\') start = p + 1;
        prog = start;
    }

    auto make_usage_line = [&]() -> std::string {
        if (!config.usage_.empty()) return config.usage_;
        std::string s = "Usage: " + prog;
        if (!config.flags_.empty() || !config.options_.empty()) s += " [options]";
        for (const auto& p : config.positionals_)
            s += p.is_list ? " [<" + p.name + ">...]" : " <" + p.name + ">";
        return s;
    };

    auto make_version = [&]() -> std::string {
        return config.version_.empty() ? prog : prog + " " + config.version_;
    };

    auto make_help = [&]() -> std::string {
        std::string out = make_usage_line();

        if (!config.flags_.empty() || !config.options_.empty()) {
            std::vector<std::string> names, descs;
            for (const auto& d : config.flags_) {
                std::string col;
                if (d.short_name != '\0') { col += '-'; col += d.short_name; }
                if (!d.long_name.empty()) { if (!col.empty()) col += ", "; col += "--"; col += d.long_name; }
                names.push_back(std::move(col));
                descs.push_back(d.description);
            }
            for (const auto& d : config.options_) {
                std::string col;
                if (d.short_name != '\0') { col += '-'; col += d.short_name; }
                if (!d.long_name.empty()) { if (!col.empty()) col += ", "; col += "--"; col += d.long_name; }
                col += " <val>";
                names.push_back(std::move(col));
                descs.push_back(d.description);
            }
            size_t w = 0;
            for (const auto& n : names) if (n.size() > w) w = n.size();
            out += "\n\nOptions:";
            for (size_t k = 0; k < names.size(); ++k)
                out += "\n  " + names[k] + std::string(w - names[k].size() + 2, ' ') + descs[k];
        }

        if (!config.positionals_.empty()) {
            std::vector<std::string> names, descs;
            for (const auto& p : config.positionals_) {
                names.push_back(p.is_list ? "[<" + p.name + ">...]" : "<" + p.name + ">");
                descs.push_back(p.description);
            }
            size_t w = 0;
            for (const auto& n : names) if (n.size() > w) w = n.size();
            out += "\n\nArguments:";
            for (size_t k = 0; k < names.size(); ++k)
                out += "\n  " + names[k] + std::string(w - names[k].size() + 2, ' ') + descs[k];
        }

        return out;
    };

    // Lambda to assign one positional token to the next available slot.
    // Captures pos_idx by reference so it advances as slots are filled.
    size_t pos_idx = 0;
    auto add_positional_token = [&](const char* val) -> int {
        if (pos_idx < config.positionals_.size()) {
            const auto& p = config.positionals_[pos_idx];
            if (!p.is_list) {
                if (args) args->args_.push_back({p.name, std::string_view(val)});
                ++pos_idx;
            } else {
                if (args) {
                    if (args->list_name_.empty()) args->list_name_ = p.name;
                    args->list_values_.push_back(std::string_view(val));
                }
                // stay on the list slot — it absorbs all remaining tokens
            }
            return EZ_CLI_OK;
        }
        if (message) { *message = "unexpected argument: "; *message += val; }
        return EZ_CLI_NO_MATCH;
    };

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

                // Look up as a value option
                bool found_option = false;
                for (const auto& d : config.options_) {
                    if (d.short_name != c) continue;
                    found_option = true;
                    std::string_view value;
                    if (*(p + 1) != '\0') {
                        value = p + 1;          // packed: -ofile
                    } else if (i + 1 < argc) {
                        value = argv[++i];      // space-separated: -o file
                    } else {
                        if (message) { *message = "option requires a value: -"; *message += c; }
                        return EZ_CLI_ERR_MISSING_VAL;
                    }
                    if (options) options->options_.push_back({d.long_name, d.short_name, value});
                    break;
                }
                if (found_option) break;  // option consumed rest of cluster; exit cluster loop

                if (message) { *message = "unknown option: -"; *message += c; }
                return EZ_CLI_NO_MATCH;
            }
        }
        else if (token[0] == '-' && token[1] == '-' && token[2] != '\0') {
            // Meta-options take priority over all other long options
            if (std::strcmp(token, "--help") == 0) {
                if (message) *message = make_help();
                return EZ_CLI_HELP_REQUESTED;
            }
            if (std::strcmp(token, "--usage") == 0) {
                if (message) *message = make_usage_line();
                return EZ_CLI_USAGE_REQUESTED;
            }
            if (std::strcmp(token, "--version") == 0) {
                if (message) *message = make_version();
                return EZ_CLI_VERSION_REQUESTED;
            }

            // Long flag or long value option: --name  or  --name=value
            const char* name_start = token + 2;
            const char* eq = std::strchr(name_start, '=');

            if (eq) {
                // --name=value  or  --name= (empty value is an error)
                std::string_view name(name_start, static_cast<size_t>(eq - name_start));
                const char* value_str = eq + 1;

                if (*value_str == '\0') {
                    if (message) { *message = "option requires a value: --"; message->append(name.data(), name.size()); }
                    return EZ_CLI_ERR_EMPTY_VAL;
                }

                bool found = false;
                for (const auto& d : config.options_) {
                    if (!d.long_name.empty() && d.long_name == name) {
                        found = true;
                        if (options) options->options_.push_back({d.long_name, d.short_name, std::string_view(value_str)});
                        break;
                    }
                }
                if (!found) {
                    if (message) { *message = "unknown option: --"; message->append(name.data(), name.size()); }
                    return EZ_CLI_NO_MATCH;
                }
            } else {
                // --name (no '=') — flag or space-separated value option
                const char* name = name_start;
                bool found = false;

                for (const auto& d : config.flags_) {
                    if (!d.long_name.empty() && d.long_name == name) {
                        found = true;
                        if (flags) flags->flags_.push_back({d.long_name, d.short_name});
                        break;
                    }
                }

                if (!found) {
                    for (const auto& d : config.options_) {
                        if (!d.long_name.empty() && d.long_name == name) {
                            found = true;
                            if (i + 1 < argc) {
                                std::string_view value = argv[++i];
                                if (options) options->options_.push_back({d.long_name, d.short_name, value});
                            } else {
                                if (message) { *message = "option requires a value: --"; *message += name; }
                                return EZ_CLI_ERR_MISSING_VAL;
                            }
                            break;
                        }
                    }
                }

                if (!found) {
                    if (message) { *message = "unknown option: --"; *message += name; }
                    return EZ_CLI_NO_MATCH;
                }
            }
        }
        else if (token[0] == '-' && token[1] == '-' && token[2] == '\0') {
            // '--' end-of-options: all remaining tokens are positionals
            ++i;
            while (i < argc) {
                int r = add_positional_token(argv[i]);
                if (r != EZ_CLI_OK) return r;
                ++i;
            }
            break;
        }
        else {
            // Positional token (no leading '-', or lone '-')
            int r = add_positional_token(token);
            if (r != EZ_CLI_OK) return r;
        }
        ++i;
    }

    return EZ_CLI_OK;
}

} // namespace ez
