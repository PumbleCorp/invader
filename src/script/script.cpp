// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include "../eprintf.hpp"
#include "../command_line_option.hpp"
#include "../version.hpp"

int main(int argc, const char **argv) {
    using namespace Invader;
    //using namespace Invader::HEK;

    struct ScriptOption {
        const char *path;
        const char *data = "data/";
        const char *tags = "tags/";
    } script_options;
    script_options.path = *argv;

    // Add our options
    std::vector<CommandLineOption> options;
    options.emplace_back("help", 'h', 0);
    options.emplace_back("info", 'i', 0);
    options.emplace_back("data", 'd', 1);
    options.emplace_back("tags", 't', 1);

    // Parse arguments
    auto remaining_options = CommandLineOption::parse_arguments<ScriptOption &>(argc, argv, options, 'h', script_options, [](char opt, const auto &arguments, ScriptOption &script_options) {
        switch(opt) {
            case 'i':
                INVADER_SHOW_INFO
                std::exit(EXIT_FAILURE);
                break;

            case 'd':
                script_options.data = arguments[0];
                break;

            case 't':
                script_options.tags = arguments[0];
                break;

            default:
                eprintf("Usage: %s <options> <scenario>\n", script_options.path);
                eprintf("Compile scripts for scenario tags.\n\n");
                eprintf("Options:\n");
                eprintf("  --help,-h                    Show this help directory.\n");
                eprintf("  --info,-i                    Show credits, source info, and other info.\n");
                eprintf("  --maps,-m <dir>              Use a specific maps directory.\n");
                eprintf("  --tags,-t <dir>              Use a specific tags directory.\n\n");
                std::exit(EXIT_FAILURE);
        }
    });

    // Get the scenario tag
    const char *scenario;
    if(remaining_options.size() == 0) {
        eprintf("Expected a scenario tag. Use -h for more information.\n");
        return EXIT_FAILURE;
    }
    else if(remaining_options.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_options[1]);
        return EXIT_FAILURE;
    }
    else {
        scenario = remaining_options[0];
    }
}