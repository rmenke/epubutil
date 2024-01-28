#ifndef _spine_hpp_
#define _spine_hpp_

#include "manifest.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <ranges>
#include <vector>

namespace epub {

class spine {
    using item_ptr = std::shared_ptr<manifest_item>;
    using weak_item_ptr = item_ptr::weak_type;

    std::vector<weak_item_ptr> _items;

    /// @cond implementation

    static constexpr auto as_shared =
        std::views::transform(&weak_item_ptr::lock);
    static constexpr auto ignore_nulls =
        std::views::filter(&item_ptr::operator bool);

    mutable decltype(_items | as_shared | ignore_nulls) _view =
        _items | as_shared | ignore_nulls;

    /// @endcond

  public:
    spine() = default;

    spine(const spine &) = delete;
    spine(spine &&) = default;

    ~spine() = default;

    spine &operator=(const spine &) = delete;
    spine &operator=(spine &&) = default;

    void add(weak_item_ptr ptr) {
        _items.push_back(std::move(ptr));
    }

    auto begin() const {
        return _view.begin();
    }
    auto end() const {
        return _view.end();
    }
};

} // namespace epub

#endif
