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
    cli::option_processor opt;

    std::optional<std::filesystem::path> basedir;
    bool omit_toc = false;

    opt.synopsis(
        std::filesystem::path{argv[0]}.filename().string() +
        " [--force] [--output output] [--basedir dir] [--title title]"
        " [--creator name [--file-as index] [--role marc]] [--collection"
        " name [--set | --series]] content-file...");

    auto config = epub::common_options(opt);

    opt.add_option(
        'b', "basedir",
        [&](const std::string &arg) {
            if (basedir.has_value()) {
                throw cli::usage_error("basedir set multiple times");
            }
            basedir = arg;
        },
        "prefix of the input files");
    opt.add_flag(
        "omit-toc", [&] { omit_toc = true; },
        "do not include the ToC in the reading order");

    auto args_begin = argv + 1;
    auto args_end = argv + argc;

    args_begin = opt.process(args_begin, args_end);

    if (args_begin == args_end) {
        std::cerr << "error: no content files specified\n" << std::endl;
        opt.usage();
        exit(1);
    }

    epub::container container{epub::container::options::omit_toc};

    auto &metadata = container.package().metadata();

    metadata.title(config->title);
    if (!config->identifier.empty()) metadata.identifier(config->identifier);
    metadata.creators() = std::move(config->creators);
    metadata.collections() = std::move(config->collections);

    for (std::string_view arg :
         std::ranges::subrange(args_begin, args_end)) {
        std::filesystem::path source, local;

        if (auto pos = arg.rfind(':'); pos != arg.npos) {
            source = arg.substr(0, pos);
            local = arg.substr(pos + 1);
        }
        else {
            source = arg;

            if (basedir.has_value()) {
                local = std::filesystem::proximate(source, *basedir);
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
        container.navigation().stylesheet(config->toc_stylesheet);
    }

    container.write(config->output);
}
