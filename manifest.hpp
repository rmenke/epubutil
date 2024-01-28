#ifndef _manifest_hpp_
#define _manifest_hpp_

#include "file_metadata.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <ranges>
#include <stdexcept>

namespace epub {

class manifest_item {
    std::u8string _id;
    std::filesystem::path _path;
    std::u8string _properties;
    file_metadata _metadata;

  public:
    manifest_item(const std::u8string &id,
                  const std::filesystem::path &path,
                  const std::u8string &properties,
                  const file_metadata &metadata)
        : _id(id)
        , _path(path)
        , _properties(properties)
        , _metadata(metadata) {}

    const auto &id() const {
        return _id;
    }
    void id(std::u8string id) {
        _id = std::move(id);
    }
    const auto &path() const {
        return _path;
    }
    const auto &properties() const {
        return _properties;
    }
    const auto &metadata() const {
        return _metadata;
    }
};

class manifest : std::vector<std::shared_ptr<manifest_item>> {
  public:
    manifest() = default;

    manifest(const manifest &) = delete;
    manifest(manifest &&) = delete;

    ~manifest() = default;

    manifest &operator=(const manifest &) = delete;
    manifest &operator=(manifest &&) = delete;

    std::shared_ptr<manifest_item> add(const std::u8string &id,
                                       const std::filesystem::path &path,
                                       const std::u8string &properties,
                                       const file_metadata &metadata) {
        auto it =
            std::make_shared<manifest_item>(id, path, properties, metadata);
        push_back(it);
        return it;
    }

    std::shared_ptr<manifest_item> add(const std::filesystem::path &path,
                                       const std::u8string &properties,
                                       const file_metadata &metadata) {
        return add(std::u8string{}, path, properties, metadata);
    }

    using vector::back;
    using vector::begin;
    using vector::end;
    using vector::front;
};

} // namespace epub

#endif
