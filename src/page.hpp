#ifndef _epub_page_hpp_
#define _epub_page_hpp_

#include "image_ref.hpp"

#include <cassert>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <vector>

namespace epub::comic {

enum class separation_mode {
    external,    ///< Place maximum space between images.
    distributed, ///< Evenly space images.
    internal,    ///< Place no space between images.
};

/// @brief A collection of images that can fit on a single content
/// page.
///
/// Images should not be added to the page if the collective height of
/// the images would exceed the page size.
///
struct page : std::vector<image_ref> {
    /// @brief The size of the virtual page.
    const geom::size &page_size; // NOLINT

    /// @brief The size of the images stacked vertically without
    /// spacing.
    geom::size content_size;

    /// @brief The relative path of the content page.
    std::filesystem::path path;

    page(const geom::size &page_size, unsigned num);

    /// @brief Adjust the frames of the image on the page.
    ///
    /// The @c mode argument determines the disposition of white space
    /// on the page.
    ///
    /// @param mode the disposition of white space on the page
    ///
    void layout(separation_mode mode);

    /// @brief Add an image to the page.
    ///
    /// If the page has enough vertical space for the image, adds the
    /// image to itself and returns @true; otherwise, returns @c
    /// false.
    ///
    /// @param image the image to add
    /// @returns true if the image was added to the page
    ///
    bool add_image(image_ref image) {
        const auto &frame = image.frame;

        assert(frame.w <= page_size.w);
        assert(frame.h <= page_size.h);

        if (content_size.h + frame.h > page_size.h) return false;

        content_size.h += frame.h;
        content_size.w = std::max(content_size.w, frame.w);

        assert(content_size.h <= page_size.h);
        assert(content_size.w <= page_size.w);

        emplace_back(std::move(image));

        return true;
    }

    std::u8string viewport() const {
        using namespace std::string_literals;

        const std::string page_width = std::to_string(page_size.w);
        const std::string page_height = std::to_string(page_size.h);

        return u8"width="s +
               reinterpret_cast<const char8_t *>(page_width.c_str()) +
               u8", height="s +
               reinterpret_cast<const char8_t *>(page_height.c_str());
    }
};

static_assert(std::ranges::range<page>);

} // namespace epub::comic

#endif
