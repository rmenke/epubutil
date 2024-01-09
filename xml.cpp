#include "xml.hpp"

#include "manifest.hpp"
#include "navigation.hpp"
#include "package.hpp"

#include <libxml/encoding.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <memory>
#include <span>
#include <sstream>
#include <string>

namespace epub::xml {

template <class T>
struct _xml_deleter;

#define DELETER(P, S)                      \
    template <>                            \
    struct _xml_deleter<P##S> {            \
        void operator()(P##S *ptr) const { \
            P##Free##S(ptr);               \
        }                                  \
    }

template <>
struct _xml_deleter<xmlChar> {
    void operator()(xmlChar *ptr) const {
        xmlFree(ptr);
    }
};

DELETER(xml, Doc);
DELETER(xml, Node);
DELETER(xmlXPath, Context);
DELETER(xmlXPath, Object);

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

void write_manifest(xmlNodePtr manifest, const class manifest &m) {
    for (auto &&item : m) {
        auto node = xmlNewChild(manifest, nullptr, UTF8("item"), nullptr);
        xmlSetProp(node, UTF8("id"), to_xmlchar(item->id()));
        xmlSetProp(node, UTF8("href"), to_xmlchar(item->path().u8string()));
        xmlSetProp(node, UTF8("media-type"),
                   to_xmlchar(item->metadata().at(u8"media-type")));
        if (auto props = item->properties(); !props.empty()) {
            xmlSetProp(node, UTF8("properties"), to_xmlchar(props));
        }
    }
}

void write_spine(xmlNodePtr spine, const class spine &s) {
    for (auto &&itemref : s) {
        auto node = xmlNewChild(spine, nullptr, UTF8("itemref"), nullptr);
        xmlSetProp(node, UTF8("idref"), to_xmlchar(itemref->id()));
    }
}

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

    auto encoding = xmlGetCharEncodingName(XML_CHAR_ENCODING_UTF8);

    if (xmlSaveFormatFileEnc(path.c_str(), doc.get(), encoding, 1) < 0) {
        throw std::filesystem::filesystem_error(
            __func__, path, std::make_error_code(std::errc::io_error));
    }
}

void write_navigation(const std::filesystem::path &path,
                      const navigation &n) {
    auto doc = new_doc();

    auto html = xmlNewDocNode(doc.get(), nullptr, UTF8("html"), nullptr);
    auto xhtml_ns = xmlNewNs(html, to_xmlchar(xhtml_ns_uri), nullptr);
    xmlSetNs(html, xhtml_ns);

    xmlDocSetRootElement(doc.get(), html);

    auto head = xmlNewChild(html, xhtml_ns, UTF8("head"), nullptr);
    auto body = xmlNewChild(html, xhtml_ns, UTF8("body"), nullptr);

    xmlNewChild(head, xhtml_ns, UTF8("title"), UTF8("Table of Contents"));

    auto nav = xmlNewChild(body, xhtml_ns, UTF8("nav"), nullptr);
    auto ol = xmlNewChild(nav, xhtml_ns, UTF8("ol"), nullptr);

    auto ops_ns = xmlNewNs(nav, to_xmlchar(ops_ns_uri), UTF8("epub"));
    xmlSetNsProp(nav, ops_ns, UTF8("type"), UTF8("toc"));

    for (auto &&item : n) {
        auto li = xmlNewChild(ol, xhtml_ns, UTF8("li"), nullptr);
        auto a = xmlNewChild(li, xhtml_ns, UTF8("a"),
                             to_xmlchar(item->metadata().at(u8"title")));
        xmlSetProp(a, UTF8("href"), to_xmlchar(item->path().u8string()));
    }

    auto encoding = xmlGetCharEncodingName(XML_CHAR_ENCODING_UTF8);

    if (xmlSaveFormatFileEnc(path.c_str(), doc.get(), encoding, 1) < 0) {
        throw std::filesystem::filesystem_error(
            __func__, path, std::make_error_code(std::errc::io_error));
    }
}

/// @brief Get the node set from an XPath object.
///
/// The node set is returned as a sized range.
///
/// @param obj a managed pointer to an XPath result
/// @return a span over the node set
///
static inline auto as_list(const xml_ptr<xmlXPathObject> &obj) {
    const auto ns = obj->nodesetval;
    return std::span(ns->nodeTab, ns->nodeNr);
}

void get_xhtml_metadata(const std::filesystem::path &path,
                        file_metadata &metadata) {
    auto doc =
        as_xml_ptr(xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOENT));
    auto ctx = as_xml_ptr(xmlXPathNewContext(doc.get()));

    xmlXPathRegisterNs(ctx.get(), UTF8("ht"), to_xmlchar(xhtml_ns_uri));

    auto result = as_xml_ptr(
        xmlXPathEval(UTF8("string(/ht:html/ht:head/ht:title)"), ctx.get()));

    auto title = reinterpret_cast<const char8_t *>(result->stringval);

    metadata[u8"title"] = title;

    result = as_xml_ptr(xmlXPathEval(
        UTF8("/ht:html/ht:head/ht:meta[starts-with(@name, 'epub:')]"),
        ctx.get()));

    for (auto &&node : as_list(result)) {
        auto namep = xmlGetProp(node, UTF8("name"));
        std::u8string name{reinterpret_cast<const char8_t *>(namep)};
        xmlFree(namep);

        auto contentp = xmlGetProp(node, UTF8("content"));
        std::u8string content{reinterpret_cast<const char8_t *>(contentp)};
        xmlFree(contentp);

        metadata[std::move(name).substr(5)] = std::move(content);
    }
}

} // namespace epub::xml
