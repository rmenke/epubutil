#include "page.hpp"

template <std::size_t N>
static inline std::string to_digits(std::unsigned_integral auto num) {
    using namespace std::views;

    std::string result(N, '0');

    for (char &ch : result | reverse) {
        ch = '0' + (num % 10);
        num /= 10;
    }

    return result;
}

namespace epub::comic {

page::page(unsigned num)
    : path("pg" + to_digits<4>(num) + ".xhtml") {}

void page::layout(separation_mode mode, const geom::size &page_size) {
    if (content_size.w > page_size.w || content_size.h > page_size.h) {
        throw std::invalid_argument{__func__};
    }

    float origin_y, y_spacing; // NOLINT

    switch (mode) {
        case separation_mode::external:
            origin_y = 0;
            y_spacing = static_cast<float>(page_size.h - content_size.h) /
                        static_cast<float>(size() - 1);
            break;
        case separation_mode::distributed:
            origin_y = y_spacing =
                static_cast<float>(page_size.h - content_size.h) /
                static_cast<float>(size() + 1);
            break;
        case separation_mode::internal:
            origin_y = static_cast<float>(page_size.h - content_size.h) / 2;
            y_spacing = 0;
            break;
        default:
            throw std::invalid_argument{__func__};
    }

    for (auto &image : static_cast<vector &>(*this)) {
        auto origin_x = (page_size.w - image.frame.w) / 2;
        image.frame = geom::point{origin_x, static_cast<size_t>(origin_y)};
        origin_y += static_cast<float>(image.frame.h);
        origin_y += y_spacing;
    }
}

} // namespace epub::comic
