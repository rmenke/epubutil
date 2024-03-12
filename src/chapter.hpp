#ifndef _chapter_hpp_
#define _chapter_hpp_

#include "logging.hpp"
#include "page.hpp"

#include <stdexcept>
#include <vector>

namespace epub::comic {

struct chapter : std::vector<page> {
    const std::u8string name;    // NOLINT
    const geom::size &page_size; // NOLINT

    template <class String>
    chapter(String &&name, const geom::size &page_size)
        : name(std::forward<String>(name))
        , page_size(page_size) {}

    void layout(separation_mode mode) {
        for (auto &&page : *this) page.layout(mode);
    }

    auto add_blank_page(unsigned page_number) {
        return emplace_back(page_size, page_number);
    }

    auto pop_blank_page() {
        if (back().empty()) pop_back();
    }

    auto &current_page() const {
        if (empty()) throw std::out_of_range{__func__};
        return back();
    }

    auto &current_page() {
        if (empty()) throw std::out_of_range{__func__};
        return back();
    }

    void add_image(image_ref image, unsigned &page_num) {
        LOG(logging::INFO, "adding ", image.path.filename(), " to ",
            reinterpret_cast<const char *>(name.c_str()));

        auto scale = image.frame.fit(page_size);

        if (scale != 1.0) {
            LOG(logging::DEBUG, "downscaling to ", scale * 100.0, "%");
            image.frame *= scale;
        }

        if (current_page().add_image(image)) return;

        LOG(logging::DEBUG, "overflow; adding new blank page");

        add_blank_page(++page_num);
        if (!current_page().add_image(image)) {
            throw std::logic_error(__func__);
        }
    }
};

} // namespace epub::comic

#endif
