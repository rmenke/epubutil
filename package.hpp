#ifndef _package_hpp_
#define _package_hpp_

#include "manifest_item.hpp"
#include "metadata.hpp"

#include <filesystem>

namespace epub {

class package {
    std::vector<manifest_item> _items;

    class metadata _metadata;

  public:
    class metadata &metadata() {
        return _metadata;
    }
    const class metadata &metadata() const {
        return _metadata;
    }

    auto manifest() const {
        return std::ranges::ref_view(_items);
    }

    auto add_to_manifest(manifest_item new_item) {
        if (new_item.id.empty()) new_item.id = generate_id();

        auto end = _items.end();

        for (auto iter = _items.begin(); iter != end; ++iter) {
            if (iter->id == new_item.id) return std::make_pair(iter, false);
        }

        _items.push_back(std::move(new_item));
        return std::make_pair(_items.end() - 1, true);
    }

    auto spine() const {
        return std::ranges::ref_view(_items) |
               std::views::filter(&manifest_item::in_spine);
    }
};

} // namespace epub

#endif
