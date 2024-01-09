#include "package.hpp"

#include <exception>
#include <filesystem>
#include <fstream>

#include "tap.hpp"

int main(int argc, const char** argv) {
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

        p.manifest().add(u8"nav", "nav.xhtml", u8"nav",
                         {{u8"media-type", u8"application/xhtml+xml"}});
        p.spine().add(p.manifest().back());

        p.write(output_file);

        ok(fs::exists(output_file), "file created");
        diag("created ", output_file);

#ifdef EPUBCHECK
        auto epubcheck_out = fs::path{output_file}.replace_extension("txt");
        auto epubcheck_cmd = EPUBCHECK " -m opf -v 3.0 -w >"s +
                             epubcheck_out.string() + " 2>&1 "s +
                             output_file.string();

        if (!eq(std::system(epubcheck_cmd.c_str()), 0)) {
            diag(epubcheck_cmd);

            std::ifstream log{epubcheck_out};
            for (std::string s; std::getline(log, s);) {
                diag("epubcheck: ", s);
            }
        }
#else
        skip(1, "epubcheck disabled");
#endif
    }
    catch (...) {
        bail_out(std::current_exception());
    }
}
