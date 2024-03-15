#ifndef _epub_constants_hpp_
#define _epub_constants_hpp_

namespace epub::comic {

/// @brief Spacing of multiple images on a page.
enum class separation_mode {
    external,    ///< Place maximum space between images.
    distributed, ///< Evenly space images.
    internal,    ///< Place no space between images.
};

} // namespace epub::comic

#endif
