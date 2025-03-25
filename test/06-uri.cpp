#include "tap.hpp"

#include "uri.hpp"

#include <exception>

namespace std {

std::ostream &operator<<(std::ostream &os, const std::u8string &s) {
    for (auto ch : s) {
        os << static_cast<char>(ch);
    }

    return os;
}

} // namespace std

int main() {
    using namespace tap;

    test_plan plan;

    try {
        {
            std::u8string input = u8"/foo/bar/baz gar.txt";

            std::u8string expected = u8"/foo/bar/baz%20gar.txt";
            std::u8string actual = uri_encoding(input);

            eq(expected, actual);
        }

        {
            std::u8string input = u8"Ægis";

            std::u8string expected = u8"%C3%86gis";
            std::u8string actual = uri_encoding(input);

            eq(expected, actual);
        }
    }
    catch (...) {
        bail_out(std::current_exception());
    }
}
