#include "container.hpp"

#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>
#include <system_error>

#include "tap.hpp"

namespace tap {

bool file_exists(const std::filesystem::path &path) {
    return ok(std::filesystem::is_regular_file(path), path, " exists");
}

} // namespace tap

int main(int argc, const char **argv) {
    using namespace tap;
    using namespace std::literals;

    namespace fs = std::filesystem;

    auto output_file = fs::path(argv[0]).filename();
    output_file.replace_extension();
    output_file = fs::temp_directory_path() / output_file;

    test_plan plan;

    try {
        std::vector<fs::path> paths;

        for (auto &&entry : fs::directory_iterator(TESTDIR)) {
            auto path = fs::relative(entry.path());
            if (path.extension() != ".xhtml") continue;
            paths.emplace_back(absolute(std::move(path)));
        }

        std::ranges::sort(paths);

        epub::container c;

        std::array<epub::creator, 1> creators{u8"Percy Marks"s};
        creators.back().file_as(u8"Marks, Percy");
        creators.back().role(u8"aut");

        c.metadata().title(u8"The Plastic Age");
        c.metadata().creators(creators);

        for (const auto &path : paths) {
            c.add(path);
        }

        try {
            c.add(paths.front());
            fail("no local duplicates");
        }
        catch (epub::duplicate_error &) {
            pass("no local duplicates");
        }

        try {
            c.add(paths.front(), "ch1-redux.xhtml");
            pass("source duplicates ok");
        }
        catch (epub::duplicate_error &) {
            fail("source duplicates ok");
        }

        fs::remove_all(output_file);

        std::ofstream{output_file};
        file_exists(output_file);

        try {
            c.write(output_file);
            fail("will not clobber");
        }
        catch (const std::system_error &ex) {
            eq(ex.code(), std::make_error_code(std::errc::file_exists),
               "will not clobber");
        }

        ok(fs::remove(output_file), "block removed");

        c.write(output_file);
        pass("container written");

        if (std::string s;
            getline(std::ifstream{output_file / "mimetype"}, s)) {
            eq(s, "application/epub+zip", "mimetype correct");
        }
        else {
            fail("mimetype correct");
        }

        file_exists(output_file / "META-INF" / "container.xml");
        file_exists(output_file / "Contents" / "package.opf");
        file_exists(output_file / "Contents" / "nav.xhtml");

        for (const auto &source : paths) {
            auto path = output_file / "Contents" / source.filename();
            file_exists(path);
        }

        file_exists(output_file / "Contents" / "ch1-redux.xhtml");

#ifdef EPUBCHECK
        auto epubcheck_out = fs::path{output_file}.replace_extension("txt");
        auto epubcheck_cmd = EPUBCHECK " -m exp -v 3.0 -w >"s +
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
    }
    catch (...) {
        bail_out(std::current_exception());
    }

    // if (plan.status() == 0) fs::remove_all(output_file);
}
