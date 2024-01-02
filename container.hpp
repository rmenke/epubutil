#ifndef _container_hpp_
#define _container_hpp_

#include "package.hpp"

#include <filesystem>
#include <map>

namespace epub {

class duplicate_error : public std::filesystem::filesystem_error {
  public:
    duplicate_error(const std::filesystem::path &p1,
                    const std::filesystem::path &p2)
        : std::filesystem::filesystem_error(
              "duplicate local name", p1, p2,
              std::make_error_code(std::errc::file_exists)) {}
};

class container {
    /// @brief Mapping from local (container) paths to source paths.
    std::map<std::filesystem::path, std::filesystem::path> _files;

    /// @brief The EPUB package document.
    class package _package;

  public:
    /// @brief Add a file to the container.
    ///
    /// Adds @p source to the container as @p local, relative to the
    /// @c "Contents" subdirectory of the container.
    ///
    /// @param source the path to the file
    /// @param local the name of the container file
    /// @throws duplicate_error if the local name is already in use
    ///
    void add(const std::filesystem::path &source,
             const std::filesystem::path &local) {
        auto key = "Contents" / local.lexically_normal();

        if (auto found = _files.find(key); found != _files.end()) {
            throw duplicate_error(source, found->second);
        }

        _files.insert({std::move(key), source.lexically_normal()});
    }

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

    void write(const std::filesystem::path &path) const;
};

} // namespace epub

#endif
