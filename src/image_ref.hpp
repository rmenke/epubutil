#ifndef _epub_image_ref_hpp_
#define _epub_image_ref_hpp_

#include "geom.hpp"

#include <filesystem>
#include <ranges>

namespace epub::comic {

struct image_info {
    std::u8string media_type;
    std::filesystem::path extension;
    geom::size size;

    explicit image_info(const std::filesystem::path &path);
};

struct image_ref {
    std::filesystem::path path;
    std::filesystem::path local;
    std::u8string media_type;
    geom::rect frame;

  private:
    image_ref(const std::filesystem::path &path,
              const std::filesystem::path &local, image_info info);

  public:
    image_ref(const std::filesystem::path &path,
              const std::filesystem::path &local);
    image_ref(const std::filesystem::path &path, unsigned num);

    std::u8string style() const;

    friend std::ostream &operator<<(std::ostream &os, const image_ref &i) {
        auto css = i.style();
        std::string style{css.begin(), css.end()};
        return os << i.path.filename() << ' ' << style;
    }
};

} // namespace epub::comic

#endif
