#ifndef _manifest_hpp_
#define _manifest_hpp_

#include "file_metadata.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <ranges>
#include <stdexcept>

namespace epub {

class id_in_use : public std::runtime_error {
    id_in_use(std::string id)
        : std::runtime_error("id in use: " + std::move(id)) {}

  public:
    id_in_use(const std::u8string &id)
        : id_in_use(std::string{id.begin(), id.end()}) {}
};

class manifest {
  public:
    class item {
        std::u8string _id;
        std::filesystem::path _path;
        std::u8string _properties;
        file_metadata _metadata;

      public:
        item(const std::u8string &id, const std::filesystem::path &path,
             const std::u8string &properties, const file_metadata &metadata)
            : _id(id)
            , _path(path)
            , _properties(properties)
            , _metadata(metadata) {}

        const auto &id() const {
            return _id;
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

  private:
    std::map<std::u8string, std::shared_ptr<item>> _items;

    /// @cond implementation

    /// NOLINTNEXTLINE
#define AUTO_MEMBER(MEMBER, INIT) decltype(INIT) MEMBER = (INIT)
    AUTO_MEMBER(_view, _items | std::views::values);
#undef AUTO_MEMBER

    /// @endcond

  public:
    manifest() = default;

    manifest(const manifest &) = delete;
    manifest(manifest &&) = delete;

    ~manifest() = default;

    manifest &operator=(const manifest &) = delete;
    manifest &operator=(manifest &&) = delete;

    std::shared_ptr<item> add(const std::u8string &id,
                              const std::filesystem::path &path,
                              const std::u8string &properties,
                              const file_metadata &metadata) {
        auto [iter, success] = _items.try_emplace(
            id, std::make_shared<item>(id, path, properties, metadata));
        if (!success) throw id_in_use(id);
        return iter->second;
    }

    auto begin() const {
        return _view.begin();
    }
    auto end() const {
        return _view.end();
    }

    auto &front() const {
        return *begin();
    }
    auto &back() const {
        return *std::prev(end());
    }

    template <class Key>
    std::shared_ptr<item> at(Key &&key) const {
        return _items.at(std::forward<Key>(key));
    }
};

} // namespace epub

#endif
