#ifndef _media_type_hpp_
#define _media_type_hpp_

#include <filesystem>
#include <iostream>
#include <map>
#include <string_view>

namespace epub {

constexpr std::u8string_view css_media_type = u8"text/css";
constexpr std::u8string_view gif_media_type = u8"image/gif";
constexpr std::u8string_view jpeg_media_type = u8"image/jpeg";
constexpr std::u8string_view otf_media_type = u8"font/otf";
constexpr std::u8string_view png_media_type = u8"image/png";
constexpr std::u8string_view svg_media_type = u8"image/svg+xml";
constexpr std::u8string_view webp_media_type = u8"image/webp";
constexpr std::u8string_view xhtml_media_type = u8"application/xhtml+xml";

static inline const std::map<std::filesystem::path, std::u8string_view>
    core_media = {
        {".css", css_media_type},     {".gif", gif_media_type},
        {".jpeg", jpeg_media_type},   {".jpg", jpeg_media_type},
        {".otf", otf_media_type},     {".png", png_media_type},
        {".svg", svg_media_type},     {".webp", webp_media_type},
        {".xhtml", xhtml_media_type},
};

/// @brief Guess the MIME type based on the file extension.
///
/// Only valid for the core media types.
///
/// @param path the path to the file
/// @returns the MIME type as a utf-8 string
/// @throws std::out_of_range if the extension does not name a known type
///
static inline std::u8string
guess_media_type(const std::filesystem::path &path) {
    return static_cast<std::u8string>(core_media.at(path.extension()));
}

} // namespace epub

#endif
