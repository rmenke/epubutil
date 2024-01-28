#include "minidom.hpp"

#include <libxml/tree.h>

#include <memory>
#include <stdexcept>

using namespace std;

namespace epub::xml {

static inline const xmlChar *xmlstr(const std::u8string &s) {
    return reinterpret_cast<const xmlChar *>(s.c_str());
}

static inline const xmlChar *xmlstr(const std::optional<std::u8string> &s) {
    return s ? reinterpret_cast<const xmlChar *>(s->c_str()) : nullptr;
}

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
    auto ptr = xmlNewDoc(xmlstr(version));
    return managed<doc>(ptr);
}

node_ptr new_node(const doc_ptr &doc, const ns_ptr &ns,
                  const std::u8string &name,
                  const std::optional<std::u8string> &content) {
    auto ptr =
        xmlNewDocNode(doc.get(), ns.get(), xmlstr(name), xmlstr(content));
    return node_ptr{doc, static_cast<node *>(ptr)};
}

ns_ptr new_ns(const node_ptr &element, const std::u8string &uri,
              const std::optional<std::u8string> &prefix) {
    auto ptr = xmlNewNs(element.get(), xmlstr(uri), xmlstr(prefix));
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
    xmlSetProp(node.get(), BAD_CAST name.c_str(), BAD_CAST value.c_str());
}

void add_child(const node_ptr &parent, const node_ptr &child) {
    xmlAddChild(parent.get(), child.get());
}

node_ptr new_child_node(const node_ptr &node, const ns_ptr &ns,
                        const std::u8string &name,
                        const std::optional<std::u8string> &content) {
    auto ptr =
        xmlNewChild(node.get(), ns.get(), xmlstr(name), xmlstr(content));
    return node_ptr{node, static_cast<struct node *>(ptr)};
}

node_ptr new_cdata(const doc_ptr &doc, const std::u8string &data) {
    auto ptr = xmlNewCDataBlock(doc.get(), BAD_CAST data.data(),
                                static_cast<int>(data.size()));
    return node_ptr{doc, static_cast<struct node *>(ptr)};
}

void save_file(const std::filesystem::path &path, const doc_ptr &doc,
               bool format) {
    xmlSaveFormatFile(path.c_str(), doc.get(), format ? 1 : 0);
}

} // namespace epub::xml
