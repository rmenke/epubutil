#ifndef _xml_hpp_
#define _xml_hpp_

#include "file_metadata.hpp"

#include <filesystem>

namespace epub {

class container;
class package;
class navigation;

namespace xml {

constexpr auto dc_ns_uri = u8"http://purl.org/dc/elements/1.1/";
constexpr auto odc_ns_uri =
    u8"urn:oasis:names:tc:opendocument:xmlns:container";
constexpr auto opf_ns_uri = u8"http://www.idpf.org/2007/opf";
constexpr auto ops_ns_uri = u8"http://www.idpf.org/2007/ops";
constexpr auto xhtml_ns_uri = u8"http://www.w3.org/1999/xhtml";

extern void write_package(const std::filesystem::path &path,
                          const package &package);

extern void write_container(const std::filesystem::path &path,
                            const container &container);

extern void get_xhtml_metadata(const std::filesystem::path &path,
                               file_metadata &metadata);

extern void get_svg_metadata(const std::filesystem::path &path,
                             file_metadata &metadata);

} // namespace xml

} // namespace epub

#endif
