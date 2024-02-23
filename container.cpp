#include "container.hpp"

#include "manifest_item.hpp"
#include "media_type.hpp"
#include "xml.hpp"

#include <fstream>

namespace fs = std::filesystem;

namespace epub {

container::container(container::options options) {
    file_metadata metadata = {
        {u8"title", u8"Table of Contents"},
        {u8"media-type", u8"application/xhtml+xml"},
    };

    const bool in_toc =
        ((options & options::omit_toc) != options::omit_toc);

    manifest_item item = {
        .id = u8"nav",
        .path = u8"nav.xhtml",
        .properties = u8"nav",
        .metadata = std::move(metadata),
        .in_spine = in_toc,
        .in_toc = in_toc,
    };

    _package.add_to_manifest(std::move(item));
}

void container::add(const std::filesystem::path &source,
                    const std::filesystem::path &local,
                    std::u8string properties) {
    auto key = "Contents" / local.lexically_normal();

    if (auto found = _files.find(key); found != _files.end()) {
        throw duplicate_error(source, found->second);
    }

    _files.emplace(std::move(key), source.lexically_normal());

    manifest_item item = {
        .path = std::move(local),
        .properties = std::move(properties),
    };

    std::u8string media_type;

    try {
        media_type = guess_media_type(item.path);
    }
    catch (const std::out_of_range &ex) {
        std::throw_with_nested(std::invalid_argument(
            __func__ + std::string{": unknown file type"}));
    }

    if (media_type == xhtml_media_type) {
        xml::get_xhtml_metadata(source, item.metadata);

        if (auto props = item.metadata.get(u8"properties"); props) {
            if (!item.properties.empty()) item.properties += u8' ';
            item.properties += *props;
        }

        if (item.metadata.get(u8"spine", u8"include") != u8"omit") {
            item.in_spine = true;
            if (item.metadata.get(u8"toc", u8"include") != u8"omit") {
                item.in_toc = true;
            }
        }
    }
    else if (media_type == svg_media_type) {
        throw std::logic_error("Not yet implemented");
        /// @todo xml::get_svg_metadata(source, metadata)

        if (auto props = item.metadata.get(u8"properties"); props) {
            if (!item.properties.empty()) item.properties += u8' ';
            item.properties += *props;
        }

        if (item.metadata.get(u8"spine", u8"omit") != u8"omit") {
            item.in_spine = true;
            if (item.metadata.get(u8"toc", u8"include") != u8"omit") {
                item.in_toc = true;
            }
        }
    }

    item.metadata[u8"media-type"] = std::move(media_type);

    _package.add_to_manifest(std::move(item));
}

template <class String>
static void write_file(const fs::path &path, String &&string) {
    if (!(std::ofstream{path} << std::forward<String>(string))) {
        throw fs::filesystem_error("unable to write", path,
                                   std::io_errc::stream);
    }
}

void container::write(const fs::path &path) const {
    if (exists(path)) {
        throw fs::filesystem_error(
            "container::write", path,
            std::make_error_code(std::errc::file_exists));
    }

    create_directory(path);

    write_file(path / "mimetype", "application/epub+zip");

    auto meta_inf_dir = path / "META-INF";
    create_directory(meta_inf_dir);

    xml::write_container(path, *this);

    for (auto &[key, source] : _files) {
        auto local = path / key;
        create_directories(local.parent_path());
        copy_file(source, local);
    }
}

} // namespace epub
