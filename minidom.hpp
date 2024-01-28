#ifndef _minidom_hpp_
#define _minidom_hpp_

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace epub::xml {

using doc_ptr = std::shared_ptr<struct doc>;
using node_ptr = std::shared_ptr<struct node>;
using ns_ptr = std::shared_ptr<struct ns>;

doc_ptr new_doc(const std::u8string &version = u8"1.0");

node_ptr
new_node(const doc_ptr &doc, const ns_ptr &ns, const std::u8string &name,
         const std::optional<std::u8string> &content = std::nullopt);

ns_ptr new_ns(const node_ptr &element, const std::u8string &uri,
              const std::optional<std::u8string> &prefix = std::nullopt);
void set_ns(const node_ptr &element, const ns_ptr &ns);

void set_root_element(const doc_ptr &doc, const node_ptr &element);
void set_attribute(const node_ptr &node, const std::u8string &name,
                   const std::u8string &value);

void add_child(const node_ptr &parent, const node_ptr &child);

node_ptr
new_child_node(const node_ptr &node, const ns_ptr &ns,
               const std::u8string &name,
               const std::optional<std::u8string> &content = std::nullopt);

node_ptr new_cdata(const doc_ptr &doc, const std::u8string &data);

void save_file(const std::filesystem::path &path, const doc_ptr &doc,
               bool format);

} // namespace epub::xml

#endif
