#ifndef _book_hpp_
#define _book_hpp_

#include "chapter.hpp"
#include "geom.hpp"

#include <vector>

namespace epub::comic {

class book : std::vector<chapter> {
    geom::size _page_size;

  public:
    using vector::begin;
    using vector::empty;
    using vector::end;

    book(geom::size page_size)
        : _page_size(std::move(page_size)) {}

    /// @brief Checked version of vector::back().
    ///
    /// @returns a reference to the last element
    /// @throws std::out_of_range if the book has no chapters
    auto &last_chapter() {
        if (empty()) throw std::out_of_range{__func__};
        return back();
    }

    auto add_chapter(std::u8string name) {
        return emplace_back(std::move(name), _page_size);
    }
};

} // namespace epub::comic

#endif
