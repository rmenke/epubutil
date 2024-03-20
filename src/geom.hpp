#ifndef _epub_geom_hpp_
#define _epub_geom_hpp_

#include <concepts>
#include <ostream>

inline namespace geom {

// clang-format off

/// @brief A point in screen (left-to-right, top-to-bottom) coordinates.
struct point {
    std::size_t x;              ///< The x coordinate.
    std::size_t y;              ///< The y coordinate.

    /// @brief Construct a point at the origin.
    point()
        : x(0), y(0) {}

    /// @brief Construct a point at the given coordinates.
    ///
    /// @param x the x coordinate
    /// @param y the y coordinate
    ///
    point(std::size_t x, std::size_t y)
        : x(x), y(y) {}

    /// @brief Equality comparison operator.
    bool operator==(const point &) const = default;

    // LCOV_EXCL_START: For diagnostics only
    /// @brief Stream output operator for points.
    friend std::ostream &operator<<(std::ostream &os, const point &p) {
        return os << '+' << p.x << '+' << p.y;
    }
    // LCOV_EXCL_STOP
};

/// @brief A size of a rectangular area.
struct size {
    std::size_t w;              ///< The width of the area.
    std::size_t h;              ///< The height of the area.

    /// @brief Construct an empty size.
    size()
        : w(0), h(0) {}

    /// @brief Construct a size with the given width and height.
    ///
    /// @param width the width of the size
    /// @param height the height of the size
    size(std::size_t width, std::size_t height)
        : w(width), h(height) {}

    /// @brief Equality comparison operator.
    ///
    bool operator==(const size &) const = default;

    /// @brief Find the scale factor for a size.
    ///
    /// Returns the largest value of @f$f@f$ such that @f$f\cdot
    /// w\le\mathtt{sz.}w@f$ and @f$f\cdot h\le\mathtt{sz.}h@f$.
    ///
    /// @param s the destination size
    ///
    /// @returns a scale factor which may be greater than zero
    ///
    double fit(const size &sz) const {
        double sw = static_cast<double>(sz.w) / static_cast<double>(w);
        double sh = static_cast<double>(sz.h) / static_cast<double>(h);
        return std::min(sw, sh);
    }

    /// @brief Scale the size.
    ///
    /// Scales the width and height by the given factor.
    ///
    /// @param scale the scaling factor
    /// @returns this size
    ///
    size &operator*=(std::floating_point auto scale) {
        w = std::round(scale * w);
        h = std::round(scale * h);

        return *this;
    }

    /// @brief Scale the size.
    ///
    /// Scales the width and height by the given factor.
    ///
    /// @param scale the scaling factor
    /// @returns the new size
    ///
    size operator*(std::floating_point auto scale) const {
        return size{*this} *= scale;
    }

    /// @brief Scale the size.
    ///
    /// Scales the width and height by the given factor.
    ///
    /// @param scale the scaling factor
    /// @param sz the size
    /// @returns the new size
    ///
    friend size operator*(std::floating_point auto scale, const size &sz) {
        return scale * sz;
    }

    // LCOV_EXCL_START: For diagnostics only
    /// @brief Stream output operator for sizes.
    friend std::ostream &operator<<(std::ostream &os, const size &sz) {
        return os << sz.w << 'x' << sz.h;
    }
    // LCOV_EXCL_STOP
};

/// A rectangle consisting of the top-left point and a size.
struct rect : point, size {
    /// @brief Construct a rectangle with the given point and size.
    ///
    /// If the size is unspecified, it uses the zero size.
    ///
    /// The point will be the top-left corner of the rectangle.  If
    /// unspecified, it uses the zero point.
    ///
    /// @param p the origin of the rectangle
    /// @param sz the size of the rectangle
    ///
    explicit rect(point p = point(), size sz = size())
        : point(p), size(sz) {}

    /// @brief Construct a rectangle with the given size.
    ///
    /// The top-left point is the zero point.
    ///
    /// @param sz the size of the rectangle
    ///
    explicit rect(size sz)
        : point(), size(sz) {}

    /// @brief Construct a rectangle specifying all four dimensions.
    ///
    /// @param x the x-coordinate of the top-left point
    /// @param y the y-coordinate of the top-left point
    /// @param w the width of the rectangle
    /// @param h the height of the rectangle
    ///
    rect(std::size_t x, std::size_t y, std::size_t w, std::size_t h)
        : point(x, y), size(w, h) {}

    /// @brief Copy constructor.
    rect(const rect &) = default;
    /// @brief Move constructor.
    rect(rect &&) = default;

    /// @brief Default destructor.
    ~rect() = default;

    /// @brief Copy assignment operator.
    rect &operator=(const rect &) = default;
    /// @brief Move assignment operator.
    rect &operator=(rect &&) = default;

    /// @brief Assign a new origin to the rectangle.
    ///
    /// @param rhs the new origin
    ///
    rect &operator=(const point &rhs) {
        static_cast<point &>(*this) = rhs;
        return *this;
    }

    /// @brief Assign a new size to the rectangle.
    ///
    /// @param rhs the new size
    ///
    rect &operator=(const size &rhs) {
        static_cast<size &>(*this) = rhs;
        return *this;
    }

    /// @brief Equality comparison operator.
    bool operator==(const rect &) const = default;

    // LCOV_EXCL_START: For diagnostics only
    /// @brief Stream output operator for rectangles.
    friend std::ostream &operator<<(std::ostream &os, const rect &r) {
        return os << static_cast<const size &>(r)
                  << static_cast<const point &>(r);
    }
    // LCOV_EXCL_STOP
};

// clang-format on

} // namespace geom

#endif
