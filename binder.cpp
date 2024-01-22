#include "container.hpp"
#include "options.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

int main(int argc, char **argv) {
    epub::container container;

    cli::option_processor opt;

    std::optional<std::filesystem::path> basedir;

    std::filesystem::path output;
    bool overwrite = false;
    std::vector<epub::creator> creators;

    opt.synopsis(std::filesystem::path{argv[0]}.filename().string() +
                 " [-f] [-o output] [-b dir] [-T title] [--creator name"
                 " [--file-as index] [--role marc]] content-file...");

    opt.add_option(
        'o', "output",
        [&](const std::string &arg) {
            if (!output.empty()) {
                throw cli::usage_error("output path set multiple times");
            }
            output = arg;
        },
        "the output path (default: \"untitled.epub\")");
    opt.add_flag(
        'f', "force", [&] { overwrite = true; },
        "allow overwriting of the output file");
    opt.add_option(
        'b', "basedir",
        [&](const std::string &arg) {
            if (basedir.has_value()) {
                throw cli::usage_error("basedir set multiple times");
            }
            basedir = arg;
        },
        "prefix of the input files");
    opt.add_option(
        'T', "title",
        [&](const std::string &arg) {
            container.package().metadata().title(
                std::u8string{arg.begin(), arg.end()});
        },
        "the title of the publication");
    opt.add_option(
        'C', "creator",
        [&](const std::string &arg) {
            creators.emplace_back(std::u8string{arg.begin(), arg.end()});
        },
        "the creator(s) of the publication");
    opt.add_option(
        "file-as",
        [&](const std::string &arg) {
            if (creators.empty()) {
                throw cli::usage_error("file-as must follow a creator");
            }
            creators.back().file_as(std::u8string{arg.begin(), arg.end()});
        },
        "string used for sorting the creator, usually \"last, first\"");
    opt.add_option(
        "role",
        [&](const std::string &arg) {
            if (creators.empty()) {
                throw cli::usage_error("role must follow a creator");
            }
            if (arg.size() != 3) {
                throw cli::usage_error("MARC roles are three letters long");
            }
            creators.back().role(std::u8string{arg.begin(), arg.end()});
        },
        "MARC role of the creator (e.g., 'aut')");

    auto args_begin = argv + 1;
    auto args_end = argv + argc;

    args_begin = opt.process(args_begin, args_end);

    if (args_begin == args_end) {
        std::cerr << "error: no content files specified\n" << std::endl;
        opt.usage();
        exit(1);
    }

    container.package().metadata().creators(std::move(creators));

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

    if (output.empty()) output = "untitled.epub";
    if (overwrite) std::filesystem::remove_all(output);

    container.write(output);
}
