#include "manifest_item.hpp"
#include "media_type.hpp"
#include "metadata.hpp"
#include "package.hpp"
#include "xml.hpp"

#include <__ranges/common_view.h>
#include <__ranges/filter_view.h>
#include <__ranges/transform_view.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <ostream>

#include "tap.hpp"

namespace std {

void sprint_one(std::ostream &os, const u8string &str) {
    os << reinterpret_cast<const char *>(str.c_str());
}

} // namespace std

int main(int, const char **argv) {
    using namespace tap;
    using namespace epub;
    using namespace std::literals;

    namespace fs = std::filesystem;

    test_plan plan;

    fs::path output_file =
        fs::temp_directory_path() /
        fs::path(argv[0]).filename().replace_extension(".opf");

    fs::remove_all(output_file);

    try {
        package p;

        p.metadata().description(
            u8"A look at college life in the 1920’s, "
            "including hazing, smoking, and petting.");

        epub::creator creator{u8"Percy Marks"s};
        creator.file_as(u8"Marks, Percy");
        creator.role(u8"aut");

        p.metadata().creators().push_back(std::move(creator));

        epub::collection collection{u8"Lost American Fiction"};
        collection.type(epub::collection::type::set);
        collection.group_position(23);

        p.metadata().collections().push_back(collection);

        epub::manifest_item item = {
            .id = u8"nav",
            .path = "nav.xhtml",
            .properties = u8"nav",
            .metadata = {{u8"media-type", u8"application/xhtml+xml"}},
            .in_spine = true,
        };

        p.add_to_manifest(std::move(item));

        xml::write_package(output_file, p);

        ok(fs::exists(output_file), "file created");
        diag("created ", output_file);

#ifdef EPUBCHECK
        auto epubcheck_out = fs::path{output_file}.replace_extension("txt");
        auto epubcheck_cmd = EPUBCHECK " -m opf -v 3.0 -w >"s +
                             epubcheck_out.string() + " 2>&1 "s +
                             output_file.string();

        diag("Running: ", epubcheck_cmd);

        if (!eq(std::system(epubcheck_cmd.c_str()), 0)) {
            std::ifstream log{epubcheck_out};
            for (std::string s; std::getline(log, s);) {
                diag("epubcheck: ", s);
            }
        }
#else
        skip(1, "epubcheck disabled");
#endif

        eq(u8"image/jpeg"s, epub::guess_media_type("foo.jpg"),
           "media type addition");

        std::cerr << "# "; // so warning is visible
        eq(u8"application/octet-stream"s, epub::guess_media_type("foo.bar"),
           "media type corner case");
    }
    catch (...) {
        bail_out(std::current_exception());
    }
}
