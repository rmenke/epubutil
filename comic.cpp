#include "container.hpp"
#include "epub_options.hpp"
#include "imageinfo/imageinfo.hpp"
#include "manifest.hpp"
#include "metadata.hpp"
#include "minidom.hpp"
#include "options.hpp"
#include "spine.hpp"
#include "xml.hpp"

#include <array>
#include <cmath>
#include <filesystem>
#include <functional>
#include <ios>
#include <iostream>
#include <numeric>
#include <ranges>
#include <regex>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

inline std::string to_digits(unsigned n, unsigned d) {
    std::string str(d, '0');

    for (char &c : str | std::views::reverse) {
        c = static_cast<char>('0' + n % 10);
        n /= 10;
    }

    return str;
}

inline namespace geom {

// clang-format off

struct point {
    std::size_t x, y;

    point() : x(0), y(0) {}
    point(std::size_t x_pos, std::size_t y_pos)
        : x(x_pos), y(y_pos) {}

    friend std::ostream &operator<<(std::ostream &os, const point &p) {
        return os << '+' << p.x << '+' << p.y;
    }
};

struct size {
    std::size_t w, h;

    size() : w(0), h(0) {}
    size(std::size_t width, std::size_t height)
        : w(width), h(height) {}

    friend std::ostream &operator<<(std::ostream &os, const size &sz) {
        return os << sz.w << 'x' << sz.h;
    }
};

struct rect : point, size {
    explicit rect(point p = point(), size sz = size())
        : point(p), size(sz) {}
    explicit rect(size sz)
        : point(), size(sz) {}
    rect(std::size_t x, std::size_t y, std::size_t w, std::size_t h)
        : point(x, y), size(w, h) {}
    rect(const rect&) = default;
    rect(rect&&) = delete;

    ~rect() = default;

    rect &operator=(const rect &) = default;
    rect &operator=(rect&&) = delete;

    rect &operator=(const point &rhs) {
        *static_cast<point *>(this) = rhs; return *this;
    }
    rect &operator=(const size &rhs) {
        *static_cast<size *>(this) = rhs; return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const rect &r) {
        return os << static_cast<const size &>(r)
                  << static_cast<const point &>(r);
    }
};

// clang-format on

} // namespace geom

struct image_ref {
    std::filesystem::path path;
    std::filesystem::path local;
    std::string media_type;
    geom::rect frame;
    bool upscaled = false;

    image_ref(const std::filesystem::path &p, unsigned num,
              const ImageInfo &info)
        : path(p)
        , local("im" + to_digits(num, 5) + '.' + info.getExt())
        , media_type(info.getMimetype())
        , frame(geom::size(info.getWidth(), info.getHeight())) {}

    image_ref(std::filesystem::path p, unsigned num)
        : image_ref(p, num, getImageInfo<IIFilePathReader>(p)) {}

    auto width() const {
        return frame.w;
    }
    auto height() const {
        return frame.h;
    }

    epub::file_metadata metadata() const {
        std::u8string type{media_type.begin(), media_type.end()};
        return epub::file_metadata{{u8"media-type", std::move(type)}};
    }

    image_ref &scale(std::size_t max_width, std::size_t max_height,
                     bool upscale) {
        double max_w = static_cast<double>(max_width);
        double max_h = static_cast<double>(max_height);
        double w = static_cast<double>(frame.w);
        double h = static_cast<double>(frame.h);

        if (upscale && (w < max_w)) {
            double scale = max_w / w;
            w *= scale;
            h *= scale;
        }

        upscaled = upscale;

        if (w > max_w) {
            double scale = max_w / w;
            w *= scale;
            h *= scale;
        }
        if (h > max_h) {
            double scale = max_h / h;
            w *= scale;
            h *= scale;
        }

        frame.w = static_cast<std::size_t>(std::min(max_w, w));
        frame.h = static_cast<std::size_t>(std::min(max_h, h));

        return *this;
    }

    auto style() const {
        std::map<std::string, std::string> css;

        std::u8string result;

        auto px_rule = [](std::u8string rule,
                          std::size_t value) -> std::u8string {
            auto v = std::to_string(value) + "px";
            auto s = u8"; " + std::move(rule) + u8": " +
                     std::u8string{v.begin(), v.end()};
            return s;
        };

        result = u8"position: absolute";
        result.append(px_rule(u8"top", frame.y));
        result.append(px_rule(u8"left", frame.x));
        result.append(px_rule(u8"width", frame.w));
        result.append(px_rule(u8"height", frame.h));

        return result;
    }

    friend std::ostream &operator<<(std::ostream &os, const image_ref &i) {
        auto css = i.style();
        std::string style{css.begin(), css.end()};
        return os << i.path.filename() << ' ' << style;
    }
};

enum class separation_mode {
    maximum,     ///< Place maximum space between images.
    distributed, ///< Evenly space images.
    minimum      ///< Place no space between images.
};

class page : std::vector<image_ref> {
    geom::size _content_size;
    std::filesystem::path _path;

  public:
    page(unsigned num)
        : _path("pg" + to_digits(num, 4) + ".xhtml") {}

    auto width() const {
        return _content_size.w;
    }
    auto height() const {
        return _content_size.h;
    }
    auto path() const {
        return _path;
    }
    epub::file_metadata metadata(const std::string &chapter) const {
        return epub::file_metadata{
            {u8"media-type", u8"application/xhtml+xml"},
            {u8"title",
             reinterpret_cast<const char8_t *>(chapter.c_str())}};
    }

    using std::vector<image_ref>::empty;

    void push_back(image_ref image) {
        _content_size.w = std::max(_content_size.w, image.width());
        _content_size.h += image.height();
        emplace_back(std::move(image));
    }

    using std::vector<image_ref>::cbegin;
    using std::vector<image_ref>::cend;

    auto begin() const {
        return cbegin();
    }
    auto end() const {
        return cend();
    }

    void layout(separation_mode mode, const geom::size &page_size) {
        if (_content_size.w > page_size.w ||
            _content_size.h > page_size.h) {
            throw std::invalid_argument{__func__};
        }

        float origin_y, y_spacing; // NOLINT

        switch (mode) {
            case separation_mode::maximum:
                origin_y = 0;
                y_spacing =
                    static_cast<float>(page_size.h - _content_size.h) /
                    static_cast<float>(size() - 1);
                break;
            case separation_mode::distributed:
                origin_y = y_spacing =
                    static_cast<float>(page_size.h - _content_size.h) /
                    static_cast<float>(size() + 1);
                break;
            case separation_mode::minimum:
                origin_y =
                    static_cast<float>(page_size.h - _content_size.h) / 2;
                y_spacing = 0;
                break;
            default:
                throw std::invalid_argument{__func__};
        }

        for (auto &image : static_cast<vector &>(*this)) {
            auto origin_x = (page_size.w - image.width()) / 2;
            image.frame =
                geom::point{origin_x, static_cast<size_t>(origin_y)};
            origin_y += static_cast<float>(image.height());
            origin_y += y_spacing;
        }
    }
};

static_assert(std::ranges::range<page>);

struct chapter : std::vector<page> {
    const std::u8string name; // NOLINT

    template <class String>
    explicit chapter(String &&name)
        : name(std::forward<String>(name)) {}

    void layout(separation_mode mode, const geom::size &page_size) {
        for (auto &&page : *this) page.layout(mode, page_size);
    }

    auto add_blank_page(unsigned page_number) {
        return emplace_back(page_number);
    }

    auto pop_blank_page() {
        if (back().empty()) pop_back();
    }

    auto &current_page() {
        if (empty()) throw std::out_of_range{__func__};
        return back();
    }
};

class book : std::vector<chapter> {
  public:
    using vector::begin;
    using vector::empty;
    using vector::end;

    /// @brief Checked version of vector::back().
    ///
    /// @returns a reference to the last element
    /// @throws std::out_of_range if the book has no chapters
    auto &last_chapter() {
        if (empty()) throw std::out_of_range{"book::last_chapter"};
        return back();
    }

    auto add_chapter(std::u8string name) {
        return emplace_back(std::move(name));
    }
};

int main(int argc, char **argv) {
    std::vector<std::string> args{argv + 1, argv + argc};

    const auto progname = std::filesystem::path{argv[0]}.filename();

    cli::option_processor opt;

    opt.synopsis(
        progname.string() + " [-o output] [-fl]" +
        " [-p WIDTHxHEIGHT | -w WIDTH -h HEIGHT] [-u] image-file...");

    auto config = epub::common_options(opt);

    geom::size page_size = {1536U, 2048U};
    bool upscale = false;
    separation_mode spacing = separation_mode::distributed;
    std::filesystem::copy_options image_copy_options =
        std::filesystem::copy_options::none;

    opt.add_flag(
        'l', "link",
        [&] {
            image_copy_options =
                std::filesystem::copy_options::create_hard_links;
        },
        "link rather than copy images into the generated EPUB");
    opt.add_option(
        'p', "page-size",
        [&](const std::string &arg) {
            std::regex re("([[:digit:]]+)[^[:digit:]]+([[:digit:]]+)");
            if (std::smatch m; std::regex_match(arg, m, re)) {
                page_size.w = std::stoul(m[1]);
                page_size.h = std::stoul(m[2]);
            }
            else {
                throw cli::usage_error("page size unrecognized");
            }
        },
        "the dimensions of the page in WxH form (default: " +
            std::to_string(page_size.w) + "x" +
            std::to_string(page_size.h) + ")");
    opt.add_option(
        'w', "page-width",
        [&](const std::string &arg) { page_size.w = std::stoul(arg); },
        "the width of the page");
    opt.add_option(
        'h', "page-height",
        [&](const std::string &arg) { page_size.h = std::stoul(arg); },
        "the height of the page");
    opt.add_flag(
        'u', "upscale", [&] { upscale = true; },
        "scale images up to fit page widths");

    args.erase(args.begin(), opt.process(args.begin(), args.end()));

    for (auto iter = args.begin(); iter != args.end(); ++iter) {
        const std::string &arg = *iter;
        if (!arg.starts_with('@')) continue;

        if (arg == "@") {
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
        --iter;
    }

    if (args.empty()) {
        std::cerr << "error: no content files specified\n" << std::endl;
        opt.usage();
        exit(1);
    }

    if (config->output.empty()) config->output = "untitled.epub";

    book the_book;

    unsigned page_num = 0U;
    unsigned img_num = 0U;

    for (std::filesystem::path path : args) {
        auto chapter_name = path.parent_path().filename().u8string();

        if (the_book.empty() ||
            the_book.last_chapter().name != chapter_name) {
            the_book.add_chapter(chapter_name);
            the_book.last_chapter().add_blank_page(++page_num);
        }

        auto &current_chapter = the_book.last_chapter();

        image_ref image{path, ++img_num};

        image.scale(page_size.w, page_size.h, upscale);

        const auto current_height = current_chapter.current_page().height();

        if (current_height + image.height() > page_size.h) {
            current_chapter.add_blank_page(++page_num);
        }

        current_chapter.current_page().push_back(std::move(image));
    }

    the_book.last_chapter().pop_blank_page();

    for (auto &&chapter : the_book) {
        for (auto &&page : chapter) {
            page.layout(spacing, page_size);
        }
    }

    // clang-format off
    const std::u8string viewport(reinterpret_cast<const char8_t *>((
        "width=" + std::to_string(page_size.w) + ", "
        "height=" + std::to_string(page_size.h)).c_str()));
    // clang-format on

    if (config->overwrite) remove_all(config->output);

    epub::container c{epub::container::options::omit_toc};
    epub::manifest &m = c.package().manifest();
    epub::spine &s = c.package().spine();
    epub::navigation &n = c.navigation();

    c.package().metadata().pre_paginated();
    c.package().metadata().creators() = std::move(config->creators);

    if (!config->title.empty()) {
        c.package().metadata().title(
            reinterpret_cast<const char8_t *>(config->title.c_str()));
    }
    if (!config->identifier.empty()) {
        c.package().metadata().identifier(
            reinterpret_cast<const char8_t *>(config->identifier.c_str()));
    }
    if (!config->toc_stylesheet.empty()) {
        n.stylesheet(config->toc_stylesheet);
    }

    for (auto &&chapter : the_book) {
        std::shared_ptr<epub::manifest::item> mark;
        for (auto &&page : chapter) {
            epub::file_metadata fm{
                {u8"title", chapter.name},
                {u8"media-type", u8"application/xhtml+xml"},
            };

            auto item =
                m.add(page.path().stem().u8string(), page.path(), u8"", fm);
            s.add(item);

            if (!mark) mark = item;

            for (auto &&image : page) {
                m.add(image.local.stem().u8string(), image.local, u8"",
                      image.metadata());
            }
        }

        n.add(mark);
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
                     image_copy_options);
            }

            save_file(content_dir / page.path(), doc, true);
        }
    }
}
