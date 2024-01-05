#include "metadata.hpp"

#include <random>

namespace epub {

std::u8string generate_uuid() {
    std::default_random_engine urbg{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> nybble{0, 15};

    std::u8string uuid;

    constexpr char8_t hex[] = u8"0123456789abcdef";

    for (std::size_t ix = 0; ix < 32; ++ix) {
        if (ix == 8 || ix == 12 || ix == 16 || ix == 20) {
            uuid.push_back('-');
        }

        if (ix == 12) {
            uuid.push_back('4'); // 0b0100
        }
        else if (ix == 16) {
            uuid.push_back(hex[(nybble(urbg) & 3) | 8]); // 0b10XX
        }
        else {
            uuid.push_back(hex[nybble(urbg)]); // 0bXXXX
        }
    }

    return uuid;
}

} // namespace epub
