#include "xml.hpp"

#include "minidom.hpp"
#include "navigation.hpp"
#include "package.hpp"

#include <cassert>
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
#include <vector>

namespace epub::xml {

static inline std::u8string generate_id() {
    static unsigned next = 0U;

    std::ostringstream out;
    out << "g" << std::setw(8) << std::setfill('0') << std::hex << ++next;
    std::string s = std::move(out).str();

    return std::u8string{s.begin(), s.end()};
}

static inline node_ptr add_refinement(node_ptr parent,
                                      const std::u8string &id,
                                      const std::u8string &property,
                                      const std::u8string &content) {
    auto meta = new_child_node(parent, nullptr, u8"meta", content);
    set_attribute(meta, u8"refines", u8'#' + id);
    set_attribute(meta, u8"property", property);
    return meta;
};

static inline node_ptr add_refinement(node_ptr parent,
                                      const std::u8string &id,
                                      const std::u8string &property,
                                      const std::u8string &content,
                                      const std::u8string scheme) {
    auto meta = add_refinement(parent, id, property, content);
    set_attribute(meta, u8"scheme", scheme);
    return meta;
};

void write_metadata(node_ptr metadata_node, const class metadata &m) {
    auto dc_ns = new_ns(metadata_node, dc_ns_uri, u8"dc");

    auto identifier = new_child_node(metadata_node, dc_ns, u8"identifier",
                                     m.identifier());
    auto title = new_child_node(metadata_node, dc_ns, u8"title", m.title());
    auto language =
        new_child_node(metadata_node, dc_ns, u8"language", m.language());

    set_attribute(identifier, u8"id", u8"pub-id");

    if (!m.description().empty()) {
        auto description =
            new_child_node(metadata_node, dc_ns, u8"description");
        auto cdata = new_cdata_child(description, m.description());
    }

    // The dcterms::modified meta property is always the current time.

    {
        time_t now = time(0);
        struct tm tm;
        gmtime_r(&now, &tm);

        std::ostringstream os;
        os << std::put_time(&tm, "%FT%TZ");
        std::string fmt = std::move(os).str();
        auto timestamp = reinterpret_cast<const char8_t *>(fmt.c_str());

        auto modified =
            new_child_node(metadata_node, nullptr, u8"meta", timestamp);
        set_attribute(modified, u8"property", u8"dcterms:modified");
    }

    for (auto &&creator : m.creators()) {
        auto node =
            new_child_node(metadata_node, dc_ns, u8"creator", creator);

        std::u8string id;

        if (auto &&role = creator.role(); !role.empty()) {
            if (id.empty()) id = generate_id();
            add_refinement(metadata_node, id, u8"role", role,
                           u8"marc:relators");
        }

        if (auto &&file_as = creator.file_as(); !file_as.empty()) {
            if (id.empty()) id = generate_id();
            add_refinement(metadata_node, id, u8"file-as", file_as);
        }

        if (!id.empty()) set_attribute(node, u8"id", id);
    }

    for (auto &&collection : m.collections()) {
        auto meta =
            new_child_node(metadata_node, nullptr, u8"meta", collection);
        set_attribute(meta, u8"property", u8"belongs-to-collection");

        std::u8string id;

        switch (collection.type()) {
            case collection::type::unspecified:
                break;
            case collection::type::series: {
                if (id.empty()) id = generate_id();
                add_refinement(metadata_node, id, u8"collection-type",
                               u8"series");
                break;
            }
            case collection::type::set: {
                if (id.empty()) id = generate_id();
                add_refinement(metadata_node, id, u8"collection-type",
                               u8"set");
                break;
            }
        }

        if (const std::u8string &position = collection.group_position();
            !position.empty()) {
            if (id.empty()) id = generate_id();
            add_refinement(metadata_node, id, u8"group-position", position);
        }

        if (!id.empty()) set_attribute(meta, u8"id", id);
    }

    {
        auto meta =
            new_child_node(metadata_node, nullptr, u8"meta", m.layout());
        set_attribute(meta, u8"property", u8"rendition:layout");
    }
}

template <class Manifest>
void write_manifest(node_ptr manifest_node, Manifest &&m) {
    for (auto &&item : m) {
        auto item_node = new_child_node(manifest_node, nullptr, u8"item");

        if (item->id().empty()) item->id(generate_id());

        set_attribute(item_node, u8"id", item->id());
        set_attribute(item_node, u8"href", item->path().u8string());
        set_attribute(item_node, u8"media-type",
                      item->metadata().at(u8"media-type"));

        if (const auto &props = item->properties(); !props.empty()) {
            set_attribute(item_node, u8"properties", props);
        }
    }
}

void write_spine(node_ptr spine, const class spine &s) {
    for (auto &&itemref : s) {
        auto node = new_child_node(spine, nullptr, u8"itemref");
        set_attribute(node, u8"idref", itemref->id());
    }
}

void write_package(const std::filesystem::path &path, const package &p) {
    auto doc = new_doc(u8"1.0");

    auto root = new_node(doc, nullptr, u8"package");
    set_root_element(doc, root);

    auto opf_ns = new_ns(root, opf_ns_uri);
    set_ns(root, opf_ns);

    set_attribute(root, u8"version", u8"3.0");
    set_attribute(root, u8"unique-identifier", u8"pub-id");

    auto metadata = new_child_node(root, opf_ns, u8"metadata");
    write_metadata(metadata, p.metadata());

    auto manifest = new_child_node(root, opf_ns, u8"manifest");
    write_manifest(manifest, p.manifest());

    auto spine = new_child_node(root, opf_ns, u8"spine");
    write_spine(spine, p.spine());

    save_file(path, doc, 1);
}

void write_navigation(const std::filesystem::path &path,
                      const navigation &n) {
    auto doc = new_doc(u8"1.0");

    auto html = new_node(doc, nullptr, u8"html");
    set_root_element(doc, html);

    auto h_ns = new_ns(html, xhtml_ns_uri);
    set_ns(html, h_ns);

    auto head = new_child_node(html, h_ns, u8"head");
    auto body = new_child_node(html, h_ns, u8"body");

    set_attribute(body, u8"class", u8"navigation");

    new_child_node(head, h_ns, u8"title", u8"Table of Contents");

    if (!n.stylesheet().empty()) {
        auto ss_link = new_child_node(head, h_ns, u8"link");
        set_attribute(ss_link, u8"rel", u8"stylesheet");
        set_attribute(ss_link, u8"type", u8"text/css");
        set_attribute(ss_link, u8"href", n.stylesheet().u8string());
    }

    new_child_node(body, h_ns, u8"h1", u8"Table of Contents");

    auto nav = new_child_node(body, h_ns, u8"nav");
    auto ops_ns = new_ns(body, ops_ns_uri, u8"epub");
    set_attribute(nav, ops_ns, u8"type", u8"toc");

    auto ol = new_child_node(nav, h_ns, u8"ol");

    for (auto &&item : n) {
        const auto &title = item->metadata().at(u8"title");
        auto href = item->path().u8string();

        auto li = new_child_node(ol, h_ns, u8"li");
        auto a = new_child_node(li, h_ns, u8"a", title);
        set_attribute(a, u8"href", href);
    }

    save_file(path, doc, 1);
}

void get_xhtml_metadata(const std::filesystem::path &path,
                        file_metadata &metadata) {
    auto doc = read_file(path);
    auto ctx = xpath::new_context(doc);

    xpath::register_ns(ctx, u8"ht", xhtml_ns_uri);

    auto result = xpath::eval(u8"string(/ht:html/ht:head/ht:title)", ctx);

    metadata[u8"title"] = get<std::u8string>(result);

    result = xpath::eval(
        u8"/ht:html/ht:head/ht:meta[starts-with(@name, 'epub:')]", ctx);

    for (auto &&node : get<std::vector<node_ptr>>(result)) {
        auto name = get_attribute(node, u8"name");
        auto content = get_attribute(node, u8"content");
        metadata[std::move(name).substr(5)] = std::move(content);
    }
}

} // namespace epub::xml
