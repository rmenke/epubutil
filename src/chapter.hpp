#ifndef _chapter_hpp_
#define _chapter_hpp_

#include "page.hpp"

#include <vector>

namespace epub::comic {

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

} // namespace epub::comic

#endif
