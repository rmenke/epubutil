#ifndef _epub_logging_hpp_
#define _epub_logging_hpp_

#include <sstream>
#include <type_traits>
namespace epub {

struct logging {
    enum level : unsigned char {
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        TRACE,
        ALL = 0,
        OFF = 255
    };

  private:
    level _max_severity = level::ALL;

    logging() {}

  public:
    static logging logger; // NOLINT

    void logmsg(enum logging::level severity, const char *file,
                unsigned line, const char *func, std::string msg) const;

    template <class... Args>
    void log(enum logging::level severity, const char *file, unsigned line,
             const char *func, Args &&...args) const {
        if (severity > _max_severity) return;

        std::ostringstream os;
        (os << ... << std::forward<Args>(args));
        auto msg = std::move(os).str();
        logmsg(severity, file, line, func, std::move(msg));
    }

    void increase_level() {
        std::underlying_type_t<level> value = _max_severity;
        if (value > 254) value = 254;
        _max_severity = static_cast<level>(value + 1);
    }
    void decrease_level() {
        std::underlying_type_t<level> value = _max_severity;
        if (value < 1) value = 1;
        _max_severity = static_cast<level>(value - 1);
    }
};

// NOLINTNEXTLINE
#define LOG(LEVEL, ...)                                      \
    ::epub::logging::logger.log((LEVEL), __FILE__, __LINE__, \
                                __func__ __VA_OPT__(, ) __VA_ARGS__)

} // namespace epub

#endif
