#include "container.hpp"
#include "epub_options.hpp"
#include "metadata.hpp"
#include "options.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

int main(int argc, char **argv) {
    cli::option_processor opt{std::filesystem::path(argv[0]).filename()};

    struct configuration : epub::configuration {
        std::filesystem::path basedir;
        bool omit_toc = false;
    };

    auto config = std::make_shared<configuration>();

    epub::common_options(opt, config);

    opt.synopsis() += " [--basedir=dir] [--omit-toc] content-file...";

    opt.add_option(
        'b', "basedir",
        [config](const std::string &arg) {
            if (!config->basedir.empty()) {
                throw cli::usage_error("basedir set multiple times");
            }
            config->basedir = arg;
        },
        "prefix of the input files");
    opt.add_flag(
        "omit-toc", [config] { config->omit_toc = true; },
        "do not include the ToC in the reading order");

    auto args_begin = argv + 1;
    auto args_end = argv + argc;

    args_begin = opt.process(args_begin, args_end);

    if (args_begin == args_end) {
        std::cerr << "error: no content files specified\n" << std::endl;
        opt.usage();
        exit(1);
    }

    auto options = epub::container::options::none;
    if (config->omit_toc) options |= epub::container::options::omit_toc;

    epub::container container{options};

    auto &metadata = container.package().metadata();

    metadata.title(std::move(config->title));
    if (!config->identifier.empty())
        metadata.identifier(std::move(config->identifier));
    metadata.creators() = std::move(config->creators);
    metadata.collections() = std::move(config->collections);
    metadata.description(std::move(config->description));

    for (std::string_view arg :
         std::ranges::subrange(args_begin, args_end)) {
        std::filesystem::path source, local;

        if (auto pos = arg.rfind(':'); pos != arg.npos) {
            source = arg.substr(0, pos);
            local = arg.substr(pos + 1);
        }
        else {
            source = arg;

            if (!config->basedir.empty()) {
                local = std::filesystem::proximate(source, config->basedir);
            }
            else {
                local = source.filename();
            }
        }

        container.add(source, local);
    }

    if (config->output.empty()) config->output = "untitled.epub";
    if (config->overwrite) std::filesystem::remove_all(config->output);

    if (!config->toc_stylesheet.empty()) {
        container.toc_stylesheet(config->toc_stylesheet);
    }

    container.write(config->output);
}