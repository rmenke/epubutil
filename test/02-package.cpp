#include "package.hpp"

#include <exception>
#include <filesystem>

#include "tap.hpp"

int main(int argc, const char** argv) {
    using namespace tap;
    using namespace epub;

    test_plan plan;

    std::filesystem::path output_file =
        std::filesystem::temp_directory_path() /
        std::filesystem::path(argv[0]).filename().replace_extension(".opf");

    std::filesystem::remove_all(output_file);

    try {
        package p;

        p.write(output_file);

        ok(std::filesystem::exists(output_file), "file created");
        diag("created ", output_file);
    }
    catch (...) {
        bail_out(std::current_exception());
    }
}
