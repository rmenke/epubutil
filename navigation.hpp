#ifndef _navigation_hpp_
#define _navigation_hpp_

#include "manifest.hpp"
#include "xml.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace epub {

class navigation {
    using item_ptr = std::shared_ptr<manifest::item>;
    using weak_item_ptr = item_ptr::weak_type;

    std::vector<weak_item_ptr> _items;

    static constexpr auto as_shared =
        std::views::transform(&weak_item_ptr::lock);
    static constexpr auto ignore_nulls =
        std::views::filter(&item_ptr::operator bool);

#define AUTO_MEMBER(MEMBER, INIT) decltype(INIT) MEMBER = (INIT)

    mutable AUTO_MEMBER(_view, _items | as_shared | ignore_nulls);

  public:
    void add(std::weak_ptr<manifest::item> ptr) {
        _items.push_back(std::move(ptr));
    }

    void write(const std::filesystem::path &path) const {
        xml::write_navigation(path, *this);
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
