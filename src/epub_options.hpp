#ifndef _epub_options_cpp_
#define _epub_options_cpp_

#include "metadata.hpp"
#include "options.hpp"

#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <regex>
#include <string>

namespace epub {

struct configuration { // NOLINT
    std::filesystem::path output;
    bool overwrite = false;
    std::u8string title;
    std::u8string identifier;
    std::filesystem::path toc_stylesheet;
    std::vector<epub::creator> creators;
    std::vector<epub::collection> collections;
    std::u8string description;
    std::filesystem::path cover_image;
    epub::orientation orientation = epub::orientation::automatic;

    configuration() = default;

    configuration(const configuration &) = delete;
    configuration(configuration &&) = delete;

    configuration &operator=(const configuration &) = delete;
    configuration &operator=(configuration &&) = delete;
};

/// @brief Common options for EPUB command-line utilities.
///
/// Adds command-line options common to EPUB command-line utilities to
/// an option processor.  The arguments passed in are stored in a
/// structure managed by a shared pointer.
///
/// @param opt a @c cli::option_processor instance
/// @param config a shared pointer to a @c configuration block
void common_options(cli::option_processor &opt,
                    std::shared_ptr<configuration> config) {
    opt.synopsis() +=
        " [--output=filename] [--force] [--title=string]"
        " [--creator=name [--file-as=sort-name] [--role=marc-code]]"
        " [--collection=group [--issue=num] [--set|--series]]"
        " [--identifier=urn] [--toc-stylesheet=path]"
        " [--description=text|--description=@file]"
        " [--cover-image=filename]";

    opt.add_option(
        'o', "output",
        [config](const std::string &arg) {
            if (!config->output.empty()) {
                throw cli::usage_error("output path set multiple times");
            }
            config->output = arg;
        },
        "the output path");
    opt.add_flag(
        'f', "force", [config] { config->overwrite = true; },
        "allow overwriting of the output file");
    opt.add_option(
        'T', "title",
        [config](const std::string &arg) {
            config->title.assign(arg.begin(), arg.end());
        },
        "the title of the publication");
    opt.add_option(
        'C', "creator",
        [config](const std::string &arg) {
            config->creators.emplace_back(
                std::u8string{arg.begin(), arg.end()});
        },
        "the creator(s) of the publication");
    opt.add_option(
        "file-as",
        [config](const std::string &arg) {
            if (config->creators.empty()) {
                throw cli::usage_error("file-as must follow a creator");
            }
            config->creators.back().file_as(
                std::u8string{arg.begin(), arg.end()});
        },
        "string used for sorting the creator, usually \"last, first\"");
    opt.add_option(
        "role",
        [config](const std::string &arg) {
            if (config->creators.empty()) {
                throw cli::usage_error("role must follow a creator");
            }
            if (!std::regex_match(arg, std::regex{"[a-z]{3}"})) {
                throw cli::usage_error("MARC roles are three letters long");
            }
            config->creators.back().role(
                std::u8string{arg.begin(), arg.end()});
        },
        "MARC role of the creator (e.g., 'aut')");
    opt.add_option(
        "collection",
        [config](const std::string &arg) {
            config->collections.emplace_back(
                std::u8string{arg.begin(), arg.end()});
        },
        "the collection to which this EPUB belongs");
    opt.add_option(
        "issue",
        [config](const std::string &arg) {
            if (config->collections.empty()) {
                throw cli::usage_error("issue must follow a collection");
            }
            config->collections.back().group_position(
                std::u8string{arg.begin(), arg.end()});
        },
        "the position of the publication within the collection");
    opt.add_flag(
        "set",
        [config] {
            if (config->collections.empty()) {
                throw cli::usage_error("set must follow a collection");
            }
            auto &collection = config->collections.back();
            if (collection.type() != epub::collection::type::unspecified) {
                throw cli::usage_error(
                    "collection types are specified once");
            }
            collection.type(epub::collection::type::set);
        },
        "the collection is a complete set");
    opt.add_flag(
        "series",
        [config] {
            if (config->collections.empty()) {
                throw cli::usage_error("series must follow a collection");
            }
            auto &collection = config->collections.back();
            if (collection.type() != epub::collection::type::unspecified) {
                throw cli::usage_error(
                    "collection types are specified once");
            }
            collection.type(epub::collection::type::series);
        },
        "the collection is an ongoing series");
    opt.add_option(
        'I', "identifier",
        [config](const std::string &arg) {
            if (!config->identifier.empty()) {
                throw cli::usage_error(
                    "only one identifier per publication");
            }
            config->identifier =
                reinterpret_cast<const char8_t *>(arg.c_str());
        },
        "the publication identifier of the EPUB (default: generate)");
    opt.add_option(
        "toc-stylesheet",
        [config](const std::string &arg) { config->toc_stylesheet = arg; },
        "optional stylesheet for the Table of Contents");
    opt.add_option(
        'D', "description",
        [config](const std::string &arg) {
            if (arg.starts_with('@')) {
                std::ifstream in{arg.substr(1)};
                for (std::string line; std::getline(in, line);) {
                    if (!config->description.empty()) {
                        config->description.append(u8"\n");
                    }
                    config->description.append(line.begin(), line.end());
                }
            }
            else {
                config->description = std::u8string{arg.begin(), arg.end()};
            }
        },
        "blurb describing the EPUB");
    opt.add_option("cover-image", [config](const std::string &arg) {
        config->cover_image = arg;
    }, "image for cover of publication");
    opt.add_flag("landscape", [config] {
        config->orientation = epub::orientation::landscape;
    }, "force content to be rendered in landscape orientation");
    opt.add_flag("portrait", [config] {
        config->orientation = epub::orientation::portrait;
    }, "force content to be rendered in portrait orientation");
}

} // namespace epub

#endif
