#include "container.hpp"
#include "options.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>

int main(int argc, char **argv) {
    epub::container container;

    cli::option_processor opt;

    std::optional<std::filesystem::path> basedir;

    std::filesystem::path output;
    bool overwrite = false;

    opt.add_option('b', "basedir", [&](const std::string &arg) {
        if (basedir.has_value()) {
            throw cli::usage_error("basedir set multiple times");
        }
        basedir = arg;
    });
    opt.add_flag('f', "force", [&] { overwrite = true; });
    opt.add_option('o', "output", [&](const std::string &arg) {
        if (!output.empty()) {
            throw cli::usage_error("output path set multiple times");
        }
        output = arg;
    });
    opt.add_option('T', "title", [&](const std::string &arg) {
        container.metadata().title(std::u8string{arg.begin(), arg.end()});
    });

    auto args_begin = argv + 1;
    auto args_end = argv + argc;

    args_begin = opt.process(args_begin, args_end);

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
