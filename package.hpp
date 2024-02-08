#ifndef _package_hpp_
#define _package_hpp_

#include "manifest_item.hpp"
#include "metadata.hpp"
#include "xml.hpp"

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

    auto add_to_manifest(manifest_item item) {
        if (item.id.empty()) item.id = generate_id();
        _items.push_back(std::move(item));
    }

    auto spine() const {
        return std::ranges::ref_view(_items) |
               std::views::filter(&manifest_item::in_spine);
    }
};

} // namespace epub

#endif
