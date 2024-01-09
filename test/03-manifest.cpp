#include "manifest.hpp"

#include <exception>
#include <filesystem>

#include "tap.hpp"

namespace tap {

template <>
void sprint_one<std::u8string>(std::ostream &os, const std::u8string &u) {
    os << std::string{u.begin(), u.end()};
}

} // namespace tap

int main(int argc, char **argv) {
    using namespace tap;
    using namespace epub;
    using namespace std::literals;

    namespace fs = std::filesystem;

    test_plan plan;

    auto output_file =
        fs::temp_directory_path() /
        fs::path(argv[0]).filename().replace_extension("opf");

    fs::remove(output_file);

    try {
        manifest m;

        ok(std::ranges::empty(m), "initially empty");
        m.add(u8"nav", "nav.xhtml", u8"", file_metadata{});

        eq(std::ranges::distance(m), 1, "item added");
        eq(m.front()->id(), u8"nav"s, "\"nav\" added");
    }
    catch (...) {
        bail_out(std::current_exception());
    }
}
