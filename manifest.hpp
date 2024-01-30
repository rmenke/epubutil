#ifndef _manifest_hpp_
#define _manifest_hpp_

#include "file_metadata.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <ranges>
#include <stdexcept>

namespace epub {

/// @brief An asset of the EPUB.
///
/// All items that are part of the EPUB must appear in the manifest
/// list.  This includes linked assets such as images, fonts, and
/// stylesheets.
///
class manifest_item {
    std::u8string _id;
    std::filesystem::path _path;
    std::u8string _properties;
    file_metadata _metadata;

  public:
    /// @brief Create a manifest item.
    ///
    /// @param path the local path relative to the package document of
    ///   the asset
    /// @param properties additional manifest properties such as "nav"
    ///   or "svg"
    /// @param metadata information extracted from the file itself
    ///
    manifest_item(const std::filesystem::path &path,
                  const std::u8string &properties,
                  const file_metadata &metadata)
        : _path(path)
        , _properties(properties)
        , _metadata(metadata) {}

    /// @brief The identifier of this item.
    ///
    /// This is initially empty.  If unset at the time of document
    /// generation, a unique identifier will be supplied.
    ///
    /// @returns the identifier string
    ///
    const std::u8string &id() const {
        return _id;
    }

    /// @brief Set the identifier of this item.
    ///
    /// This is initially empty.  If unset at the time of document
    /// generation, a unique identifier will be supplied.
    ///
    /// @param id the new identifier of the asset
    ///
    void id(std::u8string id) {
        _id = std::move(id);
    }

    /// @brief The local path of the item.
    ///
    /// The path is relative to the package document and should not go
    /// upwards.
    ///
    /// @returns a relative path
    ///
    const std::filesystem::path &path() const {
        return _path;
    }

    /// @brief Additional manifest properties.
    ///
    /// Certain content documents may require additional properties.
    /// Examples include "svg" for XHTML documents with @b embedded
    /// SVG, "scripted" for interactive pages, "mathml" for technical
    /// articles, and so on.
    ///
    /// @returns a space-separated list of additional properties
    ///
    const auto &properties() const {
        return _properties;
    }

    /// @brief Additional file properties.
    ///
    /// Some information necessary for assembling the EPUB is embedded
    /// in the files themselves.  All files will have at least the
    /// "media-type" property set, which is the MIME identifier of the
    /// file.
    ///
    /// @returns a mapping of keys to properies
    ///
    const file_metadata &metadata() const {
        return _metadata;
    }
};

/// @brief A listing of all the assets in a container.
///
/// Every asset in an EPUB (both content documents and linked files)
/// must be recorded in the manifest.  Some EPUB readers will ignore
/// non-listed items; others may flag the publication as corrupted.
///
class manifest : std::vector<std::shared_ptr<manifest_item>> {
  public:
    manifest() = default;

    /// Manifest objects cannot be copied.
    manifest(const manifest &) = delete;
    /// Manifest objects cannot be moved.
    manifest(manifest &&) = delete;

    ~manifest() = default;

    /// Manifest objects cannot be copied.
    manifest &operator=(const manifest &) = delete;
    /// Manifest objects cannot be moved.
    manifest &operator=(manifest &&) = delete;

    using vector::back;
    using vector::begin;
    using vector::end;
    using vector::front;
    using vector::push_back;
};

} // namespace epub

#endif
