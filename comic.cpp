#include "container.hpp"
#include "epub_options.hpp"
#include "file_metadata.hpp"
#include "minidom.hpp"
#include "options.hpp"
#include "src/book.hpp"
#include "src/chapter.hpp"
#include "src/geom.hpp"
#include "src/image_ref.hpp"
#include "src/logging.hpp"
#include "src/page.hpp"

using epub::comic::book;
using epub::comic::chapter;
using epub::comic::image_ref;
using epub::comic::separation_mode;

int main(int argc, char **argv) {
    cli::option_processor opt{std::filesystem::path{argv[0]}.filename()};

    struct configuration : epub::configuration {
        geom::size page_size = {1536U, 2048U};
        bool upscale = false;
        separation_mode spacing = separation_mode::distributed;
        std::filesystem::copy_options image_copy_options =
            std::filesystem::copy_options::none;
    };

    auto config = std::make_shared<configuration>();

    epub::common_options(opt, config);

    opt.synopsis() +=
        " [--verbose] [--link] [--upscale]"
        " [--page-size=WIDTHxHEIGHT | --width=WIDTH --height=HEIGHT]"
        " image-file...";

    opt.add_flag(
        'v', "verbose", [] { epub::logging::logger.increase_level(); },
        "increase verbosity "
        "(may be specified more than once)");
    opt.add_flag(
        'l', "link",
        [config] {
            config->image_copy_options =
                std::filesystem::copy_options::create_hard_links;
        },
        "link rather than copy images into the generated EPUB");
    opt.add_flag(
        'u', "upscale", [config] { config->upscale = true; },
        "scale images up to fit page widths");
    opt.add_option(
        'p', "page-size",
        [config](const std::string &arg) {
            std::regex re("([[:digit:]]+)[^[:digit:]]+([[:digit:]]+)");
            if (std::smatch m; std::regex_match(arg, m, re)) {
                config->page_size.w = std::stoul(m[1]);
                config->page_size.h = std::stoul(m[2]);
            }
            else {
                throw cli::usage_error("page size unrecognized");
            }
        },
        "the dimensions of the page in WxH form (default: " +
            std::to_string(config->page_size.w) + "x" +
            std::to_string(config->page_size.h) + ")");
    opt.add_option(
        'w', "page-width",
        [config](const std::string &arg) {
            config->page_size.w = std::stoul(arg);
        },
        "the width of the page");
    opt.add_option(
        'h', "page-height",
        [config](const std::string &arg) {
            config->page_size.h = std::stoul(arg);
        },
        "the height of the page");
    opt.add_flag(
        "pack-frames",
        [config] { config->spacing = separation_mode::external; },
        "minimize space between images");
    opt.add_flag(
        "spread-frames",
        [config] { config->spacing = separation_mode::internal; },
        "maximize space between images");

    std::vector<std::string> args(argv + 1, argv + argc);

    args.erase(args.begin(), opt.process(args.begin(), args.end()));

    for (auto iter = args.begin(); iter != args.end();) {
        const std::string &arg = *iter;

        if (!arg.starts_with('@')) {
            ++iter;
        }
        else if (arg == "@") {
            iter = args.erase(iter);
            for (std::string str; getline(std::cin, str); ++iter) {
                iter = args.insert(iter, std::move(str));
            }
        }
        else {
            std::ifstream in{arg.substr(1)};
            iter = args.erase(iter);
            for (std::string str; getline(in, str); ++iter) {
                iter = args.insert(iter, std::move(str));
            }
        }
    }

    if (args.empty()) {
        std::cerr << "error: no content files specified\n" << std::endl;
        opt.usage();
        exit(1);
    }

    if (config->output.empty()) config->output = "untitled.epub";

    book the_book{config->page_size};

    unsigned page_num = 0U;
    unsigned img_num = 0U;

    for (auto &&path : args) {
        auto chapter_name = std::filesystem::absolute(path)
                                .parent_path()
                                .filename()
                                .u8string();

        if (chapter_name.empty()) {
            throw std::runtime_error{"cannot work in root directory"};
        }

        if (the_book.empty() ||
            the_book.last_chapter().name != chapter_name) {
            the_book.add_chapter(chapter_name);
            the_book.last_chapter().add_blank_page(++page_num);
        }

        auto &current_chapter = the_book.last_chapter();

        image_ref image{path, ++img_num};

        auto scale = image.frame.fit(config->page_size);

        if (scale < 1.0 || config->upscale) {
            image.frame *= scale;
        }

        const auto current_height =
            current_chapter.current_page().content_size.h;

        if (current_height + image.frame.h > config->page_size.h) {
            current_chapter.add_blank_page(++page_num);
        }

        current_chapter.current_page().push_back(std::move(image));
    }

    the_book.last_chapter().pop_blank_page();

    for (auto &&chapter : the_book) {
        for (auto &&page : chapter) {
            page.layout(config->spacing);
        }
    }

    // clang-format off
    const std::u8string viewport(reinterpret_cast<const char8_t *>((
        "width=" + std::to_string(config->page_size.w) + ", "
        "height=" + std::to_string(config->page_size.h)).c_str()));
    // clang-format on

    if (config->overwrite) remove_all(config->output);

    epub::container c{epub::container::options::omit_toc};

    c.package().metadata().pre_paginated();
    c.package().metadata().creators() = std::move(config->creators);
    c.package().metadata().collections() = std::move(config->collections);

    if (!config->title.empty()) {
        c.package().metadata().title(
            reinterpret_cast<const char8_t *>(config->title.c_str()));
    }
    if (!config->identifier.empty()) {
        c.package().metadata().identifier(
            reinterpret_cast<const char8_t *>(config->identifier.c_str()));
    }
    if (!config->toc_stylesheet.empty()) {
        c.toc_stylesheet(config->toc_stylesheet);
    }

    const epub::file_metadata page_metadata{
        {u8"title", u8"-"}, {u8"media-type", u8"application/xhtml+xml"}};

    for (auto &&chapter : the_book) {
        bool first = true;

        for (auto &&page : chapter) {
            epub::manifest_item item = {
                .id = page.path.stem().u8string(),
                .path = page.path,
                .metadata = page_metadata,
                .in_spine = true,
            };

            if (first) {
                item.in_toc = true;
                item.metadata[u8"title"] = chapter.name;
                first = false;
            }

            c.package().add_to_manifest(std::move(item));

            for (auto &&image : page) {
                epub::manifest_item image_item = {
                    .id = image.local.stem().u8string(),
                    .path = image.local,
                    .metadata = {{u8"media-type", image.media_type}},
                };
                c.package().add_to_manifest(std::move(image_item));
            }
        }
    }

    if (!config->cover_image.empty()) {
        image_ref cover{config->cover_image, "cover"};
        c.add(cover.path, cover.local, u8"cover-image");
    }

    c.write(config->output);

    auto content_dir = config->output / "Contents";

    for (auto &&chapter : the_book) {
        for (auto &&page : chapter) {
            using namespace epub::xml;

            auto doc = new_doc();
            auto root = new_node(doc, nullptr, u8"html");

            set_ns(root, new_ns(root, xhtml_ns_uri));
            set_root_element(doc, root);

            auto head = new_child_node(root, nullptr, u8"head");
            auto body = new_child_node(root, nullptr, u8"body");

            auto title =
                new_child_node(head, nullptr, u8"title", u8"Comic Page");

            auto meta = new_child_node(head, nullptr, u8"meta");
            set_attribute(meta, u8"name", u8"viewport");
            set_attribute(meta, u8"content", viewport);

#ifdef USER_STYLE
            auto style = new_child_node(head, nullptr, u8"style");
            set_attribute(style, u8"type", u8"text/css");
            auto cdata = new_cdata(doc, u8 USER_STYLE);
            add_child(style, cdata);
#endif

            for (auto &&image : page) {
                auto img = new_child_node(body, nullptr, u8"img");
                set_attribute(img, u8"style",
                              reinterpret_cast<const char8_t *>(
                                  image.style().c_str()));
                set_attribute(img, u8"src", image.local.u8string());

                copy(image.path, content_dir / image.local,
                     config->image_copy_options);
            }

            save_file(content_dir / page.path, doc, true);
        }
    }
}
