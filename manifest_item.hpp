#ifndef _manifest_item_hpp_
#define _manifest_item_hpp_

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
struct manifest_item {
    std::u8string id;
    std::filesystem::path path;
    std::u8string properties;
    file_metadata metadata;
    bool in_spine = false;
    bool in_toc = false;
};

} // namespace epub

#endif
