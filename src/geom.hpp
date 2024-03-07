#ifndef _epub_geom_hpp_
#define _epub_geom_hpp_

#include <ostream>

inline namespace geom {

// clang-format off

struct point {
    std::size_t x, y;

    point() : x(0), y(0) {}
    point(std::size_t x_pos, std::size_t y_pos)
        : x(x_pos), y(y_pos) {}

    bool operator==(const point &rhs) const = default;

    // LCOV_EXCL_START
    friend std::ostream &operator<<(std::ostream &os, const point &p) {
        return os << '+' << p.x << '+' << p.y;
    }
    // LCOV_EXCL_STOP
};

struct size {
    std::size_t w, h;

    size() : w(0), h(0) {}
    size(std::size_t width, std::size_t height)
        : w(width), h(height) {}

    bool operator==(const size &rhs) const = default;

    // LCOV_EXCL_START
    friend std::ostream &operator<<(std::ostream &os, const size &sz) {
        return os << sz.w << 'x' << sz.h;
    }
    // LCOV_EXCL_STOP
};

struct rect : point, size {
    explicit rect(point p = point(), size sz = size())
        : point(p), size(sz) {}
    explicit rect(size sz)
        : point(), size(sz) {}
    rect(std::size_t x, std::size_t y, std::size_t w, std::size_t h)
        : point(x, y), size(w, h) {}
    rect(const rect&) = default;
    rect(rect&&) = delete;

    ~rect() = default;

    rect &operator=(const rect &) = default;
    rect &operator=(rect&&) = delete;

    rect &operator=(const point &rhs) {
        *static_cast<point *>(this) = rhs; return *this;
    }
    rect &operator=(const size &rhs) {
        *static_cast<size *>(this) = rhs; return *this;
    }

    bool operator==(const rect &rhs) const = default;

    // LCOV_EXCL_START
    friend std::ostream &operator<<(std::ostream &os, const rect &r) {
        return os << static_cast<const size &>(r)
                  << static_cast<const point &>(r);
    }
    // LCOV_EXCL_STOP
};

// clang-format on

} // namespace geom

#endif
