#include "logging.hpp"

#include <chrono>
#include <iostream>
#include <string>

namespace epub {

logging logging::logger;

static inline std::string log_level(logging::level severity) {
#define CASE(X)      \
    case logging::X: \
        return #X
    switch (severity) {
        CASE(ERROR);
        CASE(WARNING);
        CASE(INFO);
        CASE(DEBUG);
        CASE(TRACE);
        default:
            return "\"" + std::to_string(severity) + "\"";
    }
#undef CASE
}

void logging::logmsg(logging::level severity,
                     [[maybe_unused]] char const* file,
                     [[maybe_unused]] unsigned int line,
                     [[maybe_unused]] char const* func,
                     std::string msg) const {
    if (severity > _max_severity) return;

    using namespace std::chrono;

    auto now = system_clock::now();
    auto day = floor<days>(now);
    auto tod = round<milliseconds>(now - day);

    auto ymd = year_month_day(day);
    auto hms = hh_mm_ss(tod);

    std::clog << ymd << ' ' << hms << ' ';
    std::clog << log_level(severity) << ' ';
    std::clog << msg << std::endl;
}

} // namespace epub
