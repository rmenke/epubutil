#include "container.hpp"

#include <filesystem>
#include <ios>

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

    write_file(contents_dir/"package.opf", "<package version='3.0' xmlns='http://www.idpf.org/2007/opf'/>");

    for (auto &[key, source] : _files) {
        auto local = path / key;
        fs::create_directories(local.parent_path());
        fs::copy_file(source, local);
    }
}

} // namespace epub
