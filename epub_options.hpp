#ifndef _epub_options_cpp_
#define _epub_options_cpp_

#include "metadata.hpp"
#include "options.hpp"

#include <filesystem>

namespace epub {

struct configuration {
    std::filesystem::path output;
    bool overwrite = false;
    std::u8string title;
    std::u8string identifier;
    std::vector<epub::creator> creators;
    std::vector<epub::collection> collections;
};

void common_options(::cli::option_processor &opt, configuration &config) {
    opt.add_option(
        'o', "output",
        [&](const std::string &arg) {
            if (!config.output.empty()) {
                throw cli::usage_error("output path set multiple times");
            }
            config.output = arg;
        },
        "the output path (default: \"untitled.epub\")");
    opt.add_flag(
        'f', "force", [&] { config.overwrite = true; },
        "allow overwriting of the output file");
    opt.add_option(
        'T', "title",
        [&](const std::string &arg) {
            config.title = reinterpret_cast<const char8_t *>(arg.c_str());
        },
        "the title of the publication");
    opt.add_option(
        'C', "creator",
        [&](const std::string &arg) {
            config.creators.emplace_back(
                std::u8string{arg.begin(), arg.end()});
        },
        "the creator(s) of the publication");
    opt.add_option(
        "file-as",
        [&](const std::string &arg) {
            if (config.creators.empty()) {
                throw cli::usage_error("file-as must follow a creator");
            }
            config.creators.back().file_as(
                std::u8string{arg.begin(), arg.end()});
        },
        "string used for sorting the creator, usually \"last, first\"");
    opt.add_option(
        "role",
        [&](const std::string &arg) {
            if (config.creators.empty()) {
                throw cli::usage_error("role must follow a creator");
            }
            if (arg.size() != 3) {
                throw cli::usage_error("MARC roles are three letters long");
            }
            config.creators.back().role(
                std::u8string{arg.begin(), arg.end()});
        },
        "MARC role of the creator (e.g., 'aut')");
    opt.add_option(
        "collection",
        [&](const std::string &arg) {
            config.collections.emplace_back(
                std::u8string{arg.begin(), arg.end()});
        },
        "the collection to which this EPUB belongs");
    opt.add_option(
        "issue",
        [&](const std::string &arg) {
            if (config.collections.empty()) {
                throw cli::usage_error("issue must follow a collection");
            }
            config.collections.back().group_position(
                std::u8string{arg.begin(), arg.end()});
        },
        "the position of the publication within the collection");
    opt.add_flag(
        "set",
        [&]() {
            if (config.collections.empty()) {
                throw cli::usage_error("set must follow a collection");
            }
            auto &collection = config.collections.back();
            if (collection.type() != epub::collection::type::unspecified) {
                throw cli::usage_error(
                    "collection types are specified once");
            }
            collection.type(epub::collection::type::set);
        },
        "the collection is a complete set");
    opt.add_flag(
        "series",
        [&]() {
            if (config.collections.empty()) {
                throw cli::usage_error("series must follow a collection");
            }
            auto &collection = config.collections.back();
            if (collection.type() != epub::collection::type::unspecified) {
                throw cli::usage_error(
                    "collection types are specified once");
            }
            collection.type(epub::collection::type::series);
        },
        "the collection is an ongoing series");
    opt.add_option(
        'I', "identifier",
        [&](const std::string &arg) {
            if (!config.identifier.empty()) {
                throw cli::usage_error(
                    "only one identifier per publication");
            }
        },
        "the publication identifier of the EPUB (default: generate)");
}

} // namespace epub

#endif
