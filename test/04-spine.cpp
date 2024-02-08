#include "manifest_item.hpp"
#include "spine.hpp"

#include <exception>

#include "tap.hpp"

int main() {
    using namespace tap;
    using namespace epub;

    test_plan plan;

    try {
        spine s;

        epub::file_metadata md;

        auto a = std::make_shared<manifest_item>("a.xhtml", u8"", md);
        a->id = u8"a";
        auto b = std::make_shared<manifest_item>("b.xhtml", u8"", md);
        b->id = u8"b";
        auto c = std::make_shared<manifest_item>("c.xhtml", u8"", md);
        c->id = u8"c";

        s.add(a);
        s.add(b);
        s.add(c);

        eq(std::ranges::distance(s), 3, "3 items");

        auto iter = s.begin();
        ok(u8"a" == (*iter++)->id, "first item");
        ok(u8"b" == (*iter++)->id, "second item");
        ok(u8"c" == (*iter++)->id, "third item");
        ok(iter == s.end(), "at end");

        b.reset();

        eq(std::ranges::distance(s), 2, "reset item dropped");

        iter = s.begin();
        ok(u8"a" == (*iter++)->id, "first item");
        ok(u8"c" == (*iter++)->id, "third item");
        ok(iter == s.end(), "at end");
    }
    catch (...) {
        bail_out(std::current_exception());
    }
}
