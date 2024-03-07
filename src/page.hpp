#ifndef _epub_page_hpp_
#define _epub_page_hpp_

#include "image_ref.hpp"

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
    /// @brief The collective size of the images stacked vertically
    /// without spacing.
    geom::size content_size;

    /// @brief The relative path of the content page.
    std::filesystem::path path;

    page(unsigned num);

    using std::vector<image_ref>::empty;

    /// @brief Add an image to the collection.
    ///
    /// As a side effect, updates the content size.
    ///
    void push_back(image_ref image) {
        content_size.w = std::max(content_size.w, image.frame.w);
        content_size.h += image.frame.h;
        emplace_back(std::move(image));
    }

    /// @brief Adjust the frames of the image on the page.
    ///
    /// The @c mode argument determines the disposition of white space
    /// on the page.
    ///
    /// @param mode the disposition of white space on the page
    /// @param page_size the size of the page in pixels
    ///
    void layout(separation_mode mode, const geom::size &page_size);
};

static_assert(std::ranges::range<page>);

} // namespace epub::comic

#endif
