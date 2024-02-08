#ifndef _package_hpp_
#define _package_hpp_

#include "manifest_item.hpp"
#include "metadata.hpp"
#include "spine.hpp"
#include "xml.hpp"
#include <__ranges/ref_view.h>

#include <filesystem>

namespace epub {

class package {
    std::vector<std::shared_ptr<manifest_item>> _items;

    class metadata _metadata;
    class spine _spine;

  public:
    class metadata &metadata() {
        return _metadata;
    }
    const class metadata &metadata() const {
        return _metadata;
    }

    auto manifest() {
        return std::ranges::ref_view(_items);
    }
    auto manifest() const {
        return std::ranges::ref_view(_items);
    }

    auto add_to_manifest(std::shared_ptr<manifest_item> item) {
        _items.push_back(std::move(item));
    }

    class spine &spine() {
        return _spine;
    }
    const class spine &spine() const {
        return _spine;
    }

    void write(const std::filesystem::path &path) const {
        xml::write_package(path, *this);
    }
};

} // namespace epub

#endif
