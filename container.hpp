#ifndef _container_hpp_
#define _container_hpp_

#include "navigation.hpp"
#include "package.hpp"

#include <filesystem>
#include <map>
#include <type_traits>

namespace epub {

/// @brief An exception indicating a duplicate local path.
class duplicate_error : public std::filesystem::filesystem_error {
  public:
    /// @brief Construct a @c duplicate_error exception.
    ///
    /// @param p1 the source path
    /// @param p2 the local path which triggered the conflict
    ///
    duplicate_error(const std::filesystem::path &p1,
                    const std::filesystem::path &p2)
        : std::filesystem::filesystem_error(
              "duplicate local name", p1, p2,
              std::make_error_code(std::errc::file_exists)) {}
};

/// @brief The EPUB container file hierarchy.
///
/// XHTML and SVG documents added are parsed for metadata information.
/// Those @c meta elements that have names prefixed with
/// @c epub: are loaded into the @c file_metadata map
/// associated with the file.  Currently, the supported tags are:
///
/// - @c epub:spine @n
///   Controls inclusion of the item in the Spine (reading order)
///   of the publication.  Items not in the spine are
///   automatically omitted from the table of contents.
///   - @c include (default for XHTML)
///   - @c omit (default for SVG)
/// - @c epub:toc @n
///   Controls inclusion of the item in the Table of Contents.
///   - @c include (default, but see above)
///   - @c omit
/// - @c epub:properties @n
///   A space-separated list of manifest item properties controlling a
///   content document.  See
///   https://www.w3.org/TR/epub-33/#sec-item-resource-properties for
///   a list of common properties.  Common properties are @c svg for
///   XHTML with embedded SVG and @c scripted for pages with embedded
///   JavaScript.
/// .
///
/// For SVG documents, metadata is freeform and can be a mixture of
/// character data and elements with no prescribed structure.  For
/// locating the relevant elements, the parser will assume that all @c
/// meta elements from the namespace @c "http://www.w3.org/1999/xhtml"
/// in the top-level SVG @c metadata element are candidates.  The name
/// prefix is the same as XHTML.
///
class container {
    /// @brief Mapping from local (container) paths to source paths.
    std::map<std::filesystem::path, std::filesystem::path> _files;

    /// @brief The EPUB package document.
    class package _package;

    /// @brief The EPUB navigation document.
    class navigation _navigation;

  public:
    enum class options { none = 0, omit_toc = 1 };

    container() : container(options::none) {}
    container(options opts);

    /// @brief Add a file to the container.
    ///
    /// Adds @p source to the container as @p local, relative to the
    /// @c "Contents" subdirectory of the container.
    ///
    /// @param source the path to the file
    /// @param local the name of the container file
    /// @param properties additional manifest properties
    /// @throws duplicate_error if the local name is already in use
    ///
    void add(const std::filesystem::path &source,
             const std::filesystem::path &local,
             std::u8string properties = {});

    /// @brief Add a file to the container.
    ///
    /// Adds @p path to the container using its filename component as
    /// the local name.
    ///
    /// @param path the path to the file
    /// @throws duplicate_error if the local name is already in use
    ///
    void add(const std::filesystem::path &path) {
        return add(path, path.filename());
    }

    /// @brief The package document.
    auto &package() {
        return _package;
    }
    /// @brief The package document.
    const auto &package() const {
        return _package;
    }

    /// @brief The navigation document.
    auto &navigation() {
        return _navigation;
    }
    /// @brief The navigation document.
    const auto &navigation() const {
        return _navigation;
    }

    /// @brief Write the EPUB container to the given path.
    ///
    /// The full prefix of @c path must exist.
    ///
    /// @param path the name of the destination directory
    ///
    void write(const std::filesystem::path &path) const;
};

static inline container::options operator~(const container::options &a) {
    return static_cast<container::options>(
        ~static_cast<std::underlying_type_t<container::options>>(a));
}

static inline container::options operator|(container::options a,
                                           container::options b) {
    using type = std::underlying_type_t<container::options>;
    return static_cast<container::options>(static_cast<type>(a) |
                                           static_cast<type>(b));
}
static inline container::options &operator|=(container::options &a,
                                             const container::options &b) {
    return a = a | b;
}

static inline container::options operator&(container::options a,
                                           container::options b) {
    using type = std::underlying_type_t<container::options>;
    return static_cast<container::options>(static_cast<type>(a) &
                                           static_cast<type>(b));
}
static inline container::options &operator&=(container::options &a,
                                             const container::options &b) {
    return a = a & b;
}

static inline container::options operator^(container::options a,
                                           container::options b) {
    using type = std::underlying_type_t<container::options>;
    return static_cast<container::options>(static_cast<type>(a) ^
                                           static_cast<type>(b));
}
static inline container::options &operator^=(container::options &a,
                                             const container::options &b) {
    return a = a ^ b;
}

} // namespace epub

#endif
