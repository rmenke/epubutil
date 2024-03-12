#include "src/geom.hpp"

#include "tap.hpp"

int main() {
    using namespace tap;

    test_plan plan = 7;

    geom::size size{480, 640};

    auto scale = size.fit({1200, 1200});

    gt(scale, 1.00, "scale - up");

    auto scaled = size * scale;

    eq(geom::size{900, 1200}, scaled, "scaled 1");

    scale = size.fit({320, 240});

    lt(scale, 1.00, "scale - down");

    scaled = size * scale;

    eq(geom::size{180, 240}, scaled, "scaled 2");

    geom::rect rect;

    eq(geom::rect{0, 0, 0, 0}, rect, "default constructor");

    rect = geom::point{50, 75};

    eq(geom::rect{50, 75, 0, 0}, rect, "point assignment");

    rect = geom::size{120, 80};

    eq(geom::rect{50, 75, 120, 80}, rect, "size assignment");

}
