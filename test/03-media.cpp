#include "media_type.hpp"

#include <stdexcept>

#include "tap.hpp"

namespace std {

void sprint_one(ostream &os, u8string s) {
    os << reinterpret_cast<const char *>(s.c_str());
}
void sprint_one(ostream &os, u8string_view s) {
    sprint_one(os, u8string{s});
}

} // namespace std

int main() {
    using namespace tap;
    using namespace std::string_view_literals;

    test_plan plan = 4;

    eq(epub::gif_media_type, epub::guess_media_type("hello/world.gif"sv),
       "GIF");
    eq(epub::jpeg_media_type, epub::guess_media_type("hello/world.jpg"sv),
       "JPEG");
    eq(epub::jpeg_media_type, epub::guess_media_type("hello/world.jpeg"sv),
       "JPEG alternate extension");

    expected_exception<std::out_of_range>(&epub::guess_media_type,
                                          "hello/world.exe"sv);
}
