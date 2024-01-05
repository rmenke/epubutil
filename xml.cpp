#include "xml.hpp"

#include "package.hpp"

#include <libxml/tree.h>
#include <libxml/valid.h>

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

namespace epub::xml {

constexpr auto opf_ns_uri = u8"http://www.idpf.org/2007/opf";
constexpr auto dc_ns_uri = u8"http://purl.org/dc/elements/1.1/";

template <class T>
struct _xml_deleter;

#define DELETER(P, S)                      \
    template <>                            \
    struct _xml_deleter<P##S> {            \
        void operator()(P##S *ptr) const { \
            P##Free##S(ptr);               \
        }                                  \
    }

DELETER(xml, Doc);
DELETER(xml, Node);

template <class T>
using xml_ptr = std::unique_ptr<T, _xml_deleter<T>>;

template <class T>
auto as_xml_ptr(T *ptr) {
    return xml_ptr<T>(ptr);
}

static inline const xmlChar *to_xmlchar(const char8_t *s) {
    return reinterpret_cast<xmlChar const *>(s);
}

static inline const xmlChar *to_xmlchar(const std::u8string &s) {
    return to_xmlchar(s.c_str());
}

#define UTF8(X) to_xmlchar(u8##X) // NOLINT

static inline xml_ptr<xmlDoc> new_doc() {
    return as_xml_ptr(xmlNewDoc(UTF8("1.0")));
}

static inline xml_ptr<xmlNode> new_doc_node(const xml_ptr<xmlDoc> &doc,
                                            const std::u8string &name) {
    return as_xml_ptr(
        xmlNewDocNode(doc.get(), nullptr, to_xmlchar(name), nullptr));
}

static inline void doc_set_root_element(const xml_ptr<xmlDoc> &doc,
                                        xml_ptr<xmlNode> &&root) {
    xmlDocSetRootElement(doc.get(), root.release());
}

void write_metadata(xmlNodePtr metadata, const class metadata &m) {
    auto dc_ns = xmlNewNs(metadata, to_xmlchar(dc_ns_uri), UTF8("dc"));

    auto identifier =
        xmlNewDocRawNode(metadata->doc, dc_ns, UTF8("identifier"),
                         to_xmlchar(m.identifier()));
    auto title = xmlNewDocRawNode(metadata->doc, dc_ns, UTF8("title"),
                                  to_xmlchar(m.title()));
    auto language = xmlNewDocRawNode(metadata->doc, dc_ns, UTF8("language"),
                                     to_xmlchar(m.language()));

    xmlSetProp(identifier, UTF8("id"), UTF8("pub-id"));

    xmlAddChild(metadata, identifier);
    xmlAddChild(metadata, title);
    xmlAddChild(metadata, language);

    // The dcterms::modified meta property is always the current time.

    {
        time_t now = time(0);
        struct tm tm;
        gmtime_r(&now, &tm);

        std::ostringstream os;
        os << std::put_time(&tm, "%FT%TZ");
        std::string fmt = std::move(os).str();

        std::u8string timestamp{fmt.begin(), fmt.end()};

        auto modified = xmlNewDocRawNode(
            metadata->doc, nullptr, UTF8("meta"), to_xmlchar(timestamp));
        xmlSetProp(modified, UTF8("property"), UTF8("dcterms:modified"));

        xmlAddChild(metadata, modified);
    }
}

void write_manifest(xmlNodePtr manifest, const class manifest &m) {}

void write_spine(xmlNodePtr spine, const class spine &m) {}

void write_package(const std::filesystem::path &path, const package &p) {
    auto doc = new_doc();

    auto root = xmlNewDocNode(doc.get(), nullptr, UTF8("package"), nullptr);
    xmlDocSetRootElement(doc.get(), root);

    auto opf_ns = xmlNewNs(root, to_xmlchar(opf_ns_uri), nullptr);
    xmlSetNs(root, opf_ns);

    xmlSetProp(root, UTF8("version"), UTF8("3.0"));
    xmlSetProp(root, UTF8("unique-identifier"), UTF8("pub-id"));

    auto metadata = xmlNewChild(root, opf_ns, UTF8("metadata"), nullptr);
    write_metadata(metadata, p.metadata());

    auto manifest = xmlNewChild(root, opf_ns, UTF8("manifest"), nullptr);
    write_manifest(manifest, p.manifest());

    auto spine = xmlNewChild(root, opf_ns, UTF8("spine"), nullptr);
    write_spine(spine, p.spine());

    if (xmlSaveFormatFileEnc(path.c_str(), doc.get(), "utf-8", 1) < 0) {
        throw std::filesystem::filesystem_error(
            __func__, path, std::make_error_code(std::errc::io_error));
    }
}

} // namespace epub::xml
