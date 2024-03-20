#include <iostream>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#include "../imageinfo/imageinfo.hpp"
#pragma clang diagnostic pop

#include "image_ref.hpp"

#include <filesystem>
#include <stdexcept>

static inline std::string to_digits(unsigned n, unsigned d) {
    std::string str(d, '0');

    for (char &c : str | std::views::reverse) {
        c = static_cast<char>('0' + n % 10);
        n /= 10;
    }

    return str;
}

namespace epub::comic {

image_info::image_info(const std::filesystem::path &path) {
    auto info = getImageInfo<IIFilePathReader>(path, II_FORMAT_PNG);

    if (!info) throw std::runtime_error{"cannot read image file"};

    media_type = reinterpret_cast<const char8_t *>(info.getMimetype());
    extension.replace_extension(info.getExt());
    size = geom::size(info.getWidth(), info.getHeight());
}

image_ref::image_ref(const std::filesystem::path &path,
                     const std::filesystem::path &local, image_info info)
    : path(path)
    , local(local)
    , media_type(std::move(info.media_type))
    , frame(std::move(info.size)) {
    this->local.replace_extension(std::move(info.extension));
}

image_ref::image_ref(const std::filesystem::path &path,
                     const std::filesystem::path &local)
    : image_ref(path, local, image_info(path)) {}

image_ref::image_ref(const std::filesystem::path &path, unsigned num)
    : image_ref(std::move(path), "im" + to_digits(num, 5)) {}

std::u8string image_ref::style() const {
    using namespace std::literals;

    auto css = "position: absolute; top: "s + std::to_string(frame.y) +
               "px; left: "s + std::to_string(frame.x) + "px; width: "s +
               std::to_string(frame.w) + "px; height: "s +
               std::to_string(frame.h) + "px"s;

    return std::u8string{reinterpret_cast<const char8_t *>(css.c_str())};
}

} // namespace epub::comic
