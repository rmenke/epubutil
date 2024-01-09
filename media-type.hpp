#ifndef _media_type_hpp_
#define _media_type_hpp_

#include <filesystem>
#include <map>
#include <string_view>

namespace epub {

constexpr std::u8string_view css_media_type = u8"text/css";
constexpr std::u8string_view gif_media_type = u8"image/gif";
constexpr std::u8string_view jpeg_media_type = u8"image/jpeg";
constexpr std::u8string_view png_media_type = u8"image/png";
constexpr std::u8string_view svg_media_type = u8"image/svg+xml";
constexpr std::u8string_view webp_media_type = u8"image/webp";
constexpr std::u8string_view xhtml_media_type = u8"application/xhtml+xml";

static inline const std::map<std::filesystem::path, std::u8string_view>
    core_media = {
        {".css", css_media_type},
        {".gif", gif_media_type},
        {".jpeg", jpeg_media_type},
        {".png", png_media_type},
        {".svg", svg_media_type},
        {".webp", webp_media_type},
        {".xhtml", xhtml_media_type},
};

inline std::u8string guess_media_type(const std::filesystem::path &path) {
    auto ext = path.extension();
    if (auto found = core_media.find(ext); found != core_media.end()) {
        return static_cast<std::u8string>(found->second);
    }
    return u8"application/octet-stream";
}

} // namespace epub

#endif
