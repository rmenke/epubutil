#include "../src/image_ref.hpp"
#include "src/geom.hpp"

#include <__ranges/common_view.h>
#include <__ranges/split_view.h>

#include <exception>
#include <filesystem>
#include <iterator>
#include <map>
#include <regex>
#include <utility>

#include "tap.hpp"

namespace std {

ostream &operator<<(ostream &os, const u8string &s) {
    return os << reinterpret_cast<const char *>(s.c_str());
}

} // namespace std

static constexpr auto match_pair = [](auto &&match) {
    return std::make_pair(match[1], match[2]);
};

int main() {
    using namespace std::literals;
    using namespace tap;
    using namespace epub::comic;

    const std::filesystem::path testdir{TESTDIR};
    const std::filesystem::path imgfile{testdir / "img_wrong_ext.gif"};

    test_plan plan;

    try {
        image_info info{imgfile};

        eq(u8"image/png"s, info.media_type, "media_type");
        eq(u8".png"s, info.extension, "extension");
        eq(geom::size{480, 640}, info.size, "size");

        image_ref img{imgfile, 23};

        eq(imgfile, img.path, "path");
        eq("im00023.png"s, img.local, "local");
        eq(u8"image/png"s, img.media_type, "media_type");
        eq(geom::rect{0, 0, 480, 640}, img.frame, "frame");

        size page_size{1200, 1200};

        auto img1 = img;
        img1.scale_to(page_size, false);
        eq(geom::size{480, 640}, img1.frame, "frame unchanged");

        auto img2 = img;
        img2.scale_to(page_size, true);
        eq(geom::size{900, 1200}, img2.frame, "frame upscaled");

        page_size = {320, 240};

        auto img3 = img;
        img3.scale_to(page_size, false);
        eq(geom::size{180, 240}, img3.frame, "frame downscaled");

        auto css = std::string{
            reinterpret_cast<const char *>(img.style().c_str())};

        using namespace std::ranges;

        std::regex pattern{R"((\w+):\s+(\w+))"};
        auto b = std::sregex_iterator{css.begin(), css.end(), pattern};
        auto e = std::sregex_iterator{};

        auto matches = subrange(b, e) | views::transform(match_pair);

        std::map<std::string, std::string>
            properties{matches.begin(), matches.end()};

        eq("0px", properties.at("top"), "top");
        eq("0px", properties.at("left"), "left");
        eq("480px", properties.at("width"), "width");
        eq("640px", properties.at("height"), "height");
        eq("absolute", properties.at("position"), "position");
    }
    catch (...) {
        bail_out(std::current_exception());
    }
}
