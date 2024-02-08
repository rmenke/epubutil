#include "container.hpp"

#include "manifest_item.hpp"
#include "media_type.hpp"
#include "xml.hpp"

#include <fstream>

namespace fs = std::filesystem;

namespace epub {

static constexpr std::string_view container_xml =
    R"%%(<?xml version="1.0" standalone="yes"?>
<container xmlns="urn:oasis:names:tc:opendocument:xmlns:container"
           version="1.0">
  <rootfiles>
    <rootfile full-path="Contents/package.opf"
              media-type="application/oebps-package+xml"/>
  </rootfiles>
</container>
)%%";

container::container(container::options options) {
#define IS_SET(opts, flag) ((opts & options::flag) == options::flag)

    auto item = std::make_shared<manifest_item>(
        u8"nav.xhtml", u8"nav",
        file_metadata{{u8"title", u8"Table of Contents"},
                      {u8"media-type", u8"application/xhtml+xml"}});
    _package.add_to_manifest(item);

    if (!IS_SET(options, omit_toc)) {
        _package.spine().add(item);
        _navigation.add(item);
    }

#undef IS_SET
}

void container::add(const std::filesystem::path &source,
                    const std::filesystem::path &local,
                    std::u8string properties) {
    auto key = "Contents" / local.lexically_normal();

    if (auto found = _files.find(key); found != _files.end()) {
        throw duplicate_error(source, found->second);
    }

    _files.insert({std::move(key), source.lexically_normal()});

    file_metadata metadata;

    auto media_type = metadata[u8"media-type"] = guess_media_type(local);

    if (media_type == xhtml_media_type) {
        xml::get_xhtml_metadata(source, metadata);

        if (auto props = metadata.get(u8"properties"); props) {
            if (!properties.empty()) properties += u8' ';
            properties += *props;
        }

        auto item =
            std::make_shared<manifest_item>(local, properties, metadata);
        _package.add_to_manifest(item);

        if (metadata.get(u8"spine", u8"include") != u8"omit") {
            _package.spine().add(item);

            if (metadata.get(u8"toc", u8"include") != u8"omit") {
                _navigation.add(item);
            }
        }
    }
    else if (media_type == svg_media_type) {
        throw std::logic_error("Not yet implemented");
        /// @todo xml::get_svg_metadata(source, metadata)

        if (auto props = metadata.get(u8"properties"); props) {
            if (!properties.empty()) properties += u8' ';
            properties += *props;
        }

        auto item =
            std::make_shared<manifest_item>(local, properties, metadata);
        _package.add_to_manifest(item);

        if (metadata.get(u8"spine", u8"omit") != u8"omit") {
            _package.spine().add(item);

            if (metadata.get(u8"toc", u8"include") != u8"omit") {
                _navigation.add(item);
            }
        }
    }
    else {
        auto item =
            std::make_shared<manifest_item>(local, properties, metadata);
        _package.add_to_manifest(item);
    }
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

    fs::create_directory(path);

    write_file(path / "mimetype", "application/epub+zip");

    auto meta_inf_dir = path / "META-INF";
    fs::create_directory(meta_inf_dir);

    write_file(meta_inf_dir / "container.xml", container_xml);

    auto contents_dir = path / "Contents";
    fs::create_directory(contents_dir);

    _package.write(contents_dir / "package.opf");

    _navigation.write(contents_dir / "nav.xhtml");

    for (auto &[key, source] : _files) {
        auto local = path / key;
        fs::create_directories(local.parent_path());
        fs::copy_file(source, local);
    }
}

} // namespace epub
