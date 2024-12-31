#include "minidom.hpp"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace epub::xml {

struct doc : xmlDoc {
    static constexpr auto deleter = &xmlFreeDoc;
};
struct node : xmlNode {
    static constexpr auto deleter = &xmlFreeNode;
};
struct ns : xmlNs {
    static constexpr auto deleter = &xmlFreeNs;
};

template <class T>
std::shared_ptr<T> managed(void *ptr) {
    return std::shared_ptr<T>(static_cast<T *>(ptr), T::deleter);
}

doc_ptr new_doc(const std::u8string &version) {
    auto v = BAD_CAST(version.c_str());

    auto ptr = xmlNewDoc(v);
    return managed<doc>(ptr);
}

node_ptr new_node(const doc_ptr &doc, const ns_ptr &ns,
                  const std::u8string &name,
                  const std::optional<std::u8string> &content) {
    auto n = BAD_CAST(name.c_str());
    auto c = BAD_CAST(content ? content->c_str() : nullptr);

    auto ptr = xmlNewDocRawNode(doc.get(), ns.get(), n, c);
    return node_ptr{doc, static_cast<node *>(ptr)};
}

ns_ptr new_ns(const node_ptr &element, const std::u8string &uri,
              const std::optional<std::u8string> &prefix) {
    auto u = BAD_CAST(uri.c_str());
    auto p = BAD_CAST(prefix ? prefix->c_str() : nullptr);

    auto ptr = xmlNewNs(element.get(), u, p);
    return ns_ptr{element, static_cast<ns *>(ptr)};
}

void set_ns(const node_ptr &element, const ns_ptr &ns) {
    xmlSetNs(element.get(), ns.get());
}

void set_root_element(const doc_ptr &doc, const node_ptr &element) {
    auto old_root = xmlDocSetRootElement(doc.get(), element.get());
    if (old_root) throw std::runtime_error{__func__};
}

void set_attribute(const node_ptr &node, const std::u8string &name,
                   const std::u8string &value) {
    auto n = BAD_CAST(name.c_str());
    auto v = BAD_CAST(value.c_str());

    xmlSetProp(node.get(), n, v);
}

void set_attribute(const node_ptr &node, const ns_ptr &ns,
                   const std::u8string &name, const std::u8string &value) {
    auto n = BAD_CAST(name.c_str());
    auto v = BAD_CAST(value.c_str());

    xmlSetNsProp(node.get(), ns.get(), n, v);
}

struct xml_free_deleter {
    void operator()(xmlChar *ptr) const {
        xmlFree(ptr);
    }
};

std::u8string get_attribute(const node_ptr &node,
                            const std::u8string &name) {
    auto n = BAD_CAST(name.c_str());

    std::unique_ptr<xmlChar, xml_free_deleter> prop{
        xmlGetProp(node.get(), n)};
    if (prop) return reinterpret_cast<const char8_t *>(prop.get());
    return {};
}

std::u8string get_attribute(const node_ptr &, const ns_ptr &,
                            const std::u8string &) {
    throw std::logic_error(std::string{__func__} + " not implemented");
}

void add_child(const node_ptr &parent, const node_ptr &child) {
    xmlAddChild(parent.get(), child.get());
}

node_ptr new_child_node(const node_ptr &node, const ns_ptr &ns,
                        const std::u8string &name,
                        const std::optional<std::u8string> &content) {
    auto n = BAD_CAST(name.c_str());
    auto c = BAD_CAST(content ? content->c_str() : nullptr);

    auto ptr = xmlNewTextChild(node.get(), ns.get(), n, c);
    return node_ptr{node, static_cast<struct node *>(ptr)};
}

node_ptr new_cdata(const doc_ptr &doc, const std::u8string &data) {
    std::span d{BAD_CAST(data.data()), data.size()};
    auto ptr = xmlNewCDataBlock(doc.get(), d.data(), d.size()); // NOLINT
    return node_ptr{doc, static_cast<struct node *>(ptr)};
}

node_ptr new_cdata_child(const node_ptr &node, const std::u8string &data) {
    std::span d{BAD_CAST(data.data()), data.size()};
    auto ptr = xmlNewCDataBlock(node->doc, d.data(), d.size()); // NOLINT
    xmlAddChild(node.get(), ptr);
    return node_ptr{node, static_cast<struct node *>(ptr)};
}

doc_ptr read_file(const std::filesystem::path &path) {
    static constexpr auto options = XML_PARSE_NOENT;
    return managed<doc>(xmlReadFile(path.c_str(), nullptr, options));
}

void save_file(const std::filesystem::path &path, const doc_ptr &doc,
               bool format) {
    xmlSaveFormatFile(path.c_str(), doc.get(), format ? 1 : 0);
}

namespace xpath {

struct context : xmlXPathContext {
    static constexpr auto deleter = &xmlXPathFreeContext;
};

struct object : xmlXPathObject {
    static constexpr auto deleter = &xmlXPathFreeObject;
};

context_ptr new_context(const doc_ptr &doc) {
    return managed<context>(xmlXPathNewContext(doc.get()));
}

void register_ns(const context_ptr &ctx, const std::u8string &prefix,
                 const std::u8string &uri) {
    xmlXPathRegisterNs(ctx.get(), BAD_CAST prefix.c_str(),
                       BAD_CAST uri.c_str());
}

using object_ptr = std::shared_ptr<struct object>;

static inline std::span<xmlNodePtr>
nodeset(const std::shared_ptr<xmlXPathObject> &obj) {
    auto nodes = obj->nodesetval;
    return nodes ? std::span(nodes->nodeTab, nodes->nodeNr)
                 : std::span<xmlNodePtr>();
}

result eval(const std::u8string &expr, const context_ptr &ctx) {
    auto e = BAD_CAST(expr.c_str());

    auto obj = managed<object>(xmlXPathEval(e, ctx.get()));

    switch (obj->type) {
        case XPATH_NODESET: {
            std::vector<node_ptr> result;

            for (xmlNodePtr n : nodeset(obj)) {
                result.emplace_back(ctx, static_cast<node *>(n));
            }

            return result;
        }
        case XPATH_STRING: {
            auto sv = reinterpret_cast<const char8_t *>(obj->stringval);
            return sv ? std::u8string{sv} : std::u8string{};
        }
        default: {
            throw std::logic_error("unsupported result type: " +
                                   std::to_string(obj->type));
        }
    }
}

} // namespace xpath

} // namespace epub::xml
