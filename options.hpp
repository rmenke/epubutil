#ifndef _options_hpp_
#define _options_hpp_

#include <__ranges/elements_view.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <source_location>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace cli {

struct option_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct option_not_found : option_error {
    template <class StringLike>
    option_not_found(StringLike &&s)
        : option_error(std::string{"option not found: "} +
                       std::string{std::forward<StringLike>(s)}) {}
};

class ambiguous_option : public option_error {
    template <class Range>
    std::string as_enumeration(Range &&r) {
        if (auto sz = std::ranges::distance(r); sz == 1) {
            return *std::ranges::begin(r);
        }
        else if (sz == 2) {
            auto iter = std::ranges::begin(r);
            return *iter + std::string{" and "} + *std::ranges::next(iter);
        }
        else {
            std::string result;
            for (auto &&opt : std::ranges::take_view(r, sz - 1)) {
                result += opt + std::string{", "};
            }
            return result + "and " + *std::ranges::rbegin(r);
        }
    }

  public:
    template <class StringLike, class Range>
    ambiguous_option(StringLike &&s, Range &&r)
        : option_error(std::string{"multiple matches for \""} +
                       std::string{std::forward<StringLike>(s)} +
                       "\": " + as_enumeration(std::forward<Range>(r))) {}

    template <class StringLike, class Iterator, class Sentinel>
    ambiguous_option(StringLike &&s, Iterator begin, Sentinel end)
        : ambiguous_option(std::forward<StringLike>(s),
                           std::ranges::subrange(begin, end)) {}
};

struct argument_needed : option_error {
    template <class StringLike>
    argument_needed(StringLike &&s)
        : option_error(std::string{std::forward<StringLike>(s)} +
                       ": argument missing") {}
};

struct argument_not_needed : option_error {
    template <class StringLike>
    argument_not_needed(StringLike &&s)
        : option_error(std::string{std::forward<StringLike>(s)} +
                       ": unexpected argument") {}
};

struct usage_error : option_error {
    using option_error::option_error;
};

class option {
  public:
    option() {}

    option(const option &) = default;
    option(option &&) = default;

    option &operator=(const option &) = default;
    option &operator=(option &&) = default;

    virtual ~option() = default;

    virtual bool needs_arg() const noexcept = 0;
    virtual void set() const {}
    virtual void set_arg(std::string) const {}
};

class flag_option : public option {
    std::function<void()> _cb;

  public:
    template <class Callback>
    flag_option(Callback &&cb)
        : _cb(std::forward<Callback>(cb)) {}

    bool needs_arg() const noexcept override {
        return false;
    }
    void set() const override {
        _cb();
    }
};

class arg_option : public option {
    std::function<void(std::string)> _cb;

  public:
    template <class Callback>
    arg_option(Callback &&cb)
        : _cb(std::forward<Callback>(cb)) {}

    bool needs_arg() const noexcept override {
        return true;
    }
    void set_arg(std::string arg) const override {
        _cb(std::move(arg));
    }
};

class option_processor {
    std::string _synopsis;
    std::vector<std::pair<std::string, std::string>> _usage;

    std::map<char, std::shared_ptr<option>> _short;
    std::map<std::string, std::shared_ptr<option>> _long;

  public:
    void synopsis(std::string synopsis) {
        _synopsis = std::move(synopsis);
    }

    template <class Callback>
    void add_flag(char ch, std::string longopt, Callback &&cb,
                  std::string description) {
        if (ch == 0 && longopt.empty()) {
            throw std::invalid_argument{"add_option"};
        }

        auto &usage =
            _usage.emplace_back(std::string{}, std::move(description));
        auto option =
            std::make_shared<flag_option>(std::forward<Callback>(cb));

        if (!longopt.empty()) {
            usage.first = "--" + longopt;
            _long[std::move(longopt)] = option;
        }

        if (ch) {
            if (!usage.first.empty()) usage.first.append(", ");
            usage.first.append(std::string("-") + ch);
            _short[ch] = std::move(option);
        }
    }

    template <class Callback>
    void add_flag(char ch, Callback &&cb, std::string description) {
        add_flag(ch, std::string{}, std::forward<Callback>(cb),
                 std::move(description));
    }

    template <class Callback>
    void add_flag(std::string longopt, Callback &&cb,
                  std::string description) {
        add_flag(0, std::move(longopt), std::forward<Callback>(cb),
                 std::move(description));
    }

    template <class Callback>
    void add_option(char ch, std::string longopt, Callback &&cb,
                    std::string description) {
        if (ch == 0 && longopt.empty()) {
            throw std::invalid_argument{"add_option"};
        }

        auto &usage =
            _usage.emplace_back(std::string{}, std::move(description));
        auto option =
            std::make_shared<arg_option>(std::forward<Callback>(cb));

        if (!longopt.empty()) {
            usage.first = "--" + longopt;
            _long[std::move(longopt)] = option;
        }

        if (ch) {
            if (!usage.first.empty()) usage.first.append(", ");
            usage.first.append(std::string("-") + ch);
            _short[ch] = std::move(option);
        }
    }

    template <class Callback>
    void add_option(char ch, Callback &&cb, std::string description) {
        add_option(ch, std::string{}, std::forward<Callback>(cb),
                   std::move(description));
    }

    template <class Callback>
    void add_option(std::string longopt, Callback &&cb,
                    std::string description) {
        add_option(0, std::move(longopt), std::forward<Callback>(cb),
                   std::move(description));
    }

    template <class ForwardIt>
    ForwardIt process(ForwardIt first, ForwardIt last) try {
        using namespace std;

        while (first != last) {
            string_view str = *first;

            if (!str.starts_with('-')) return first;
            if (str == "--"sv) return next(first);

            if (str.starts_with("--"sv)) {
                std::string arg;

                str.remove_prefix(2);

                if (auto offset = str.find('='); offset != str.npos) {
                    arg = str.substr(offset + 1);
                    str = str.substr(0, offset);
                }

                auto opt_begin = _long.lower_bound(std::string{str});
                auto opt_end = opt_begin;

                while (opt_end != _long.end() &&
                       opt_end->first.starts_with(str)) {
                    ++opt_end;
                }

                if (opt_begin == opt_end) {
                    throw option_not_found("--"s + std::string{str});
                }
                if (distance(opt_begin, opt_end) > 1) {
                    throw ambiguous_option(
                        str, std::ranges::subrange(opt_begin, opt_end) |
                                 std::views::keys |
                                 std::views::transform([](auto &&s) {
                                     return std::string{"--"} + s;
                                 }));
                }

                const auto &option = opt_begin->second;

                if (option->needs_arg()) {
                    if (arg.empty()) {
                        if (++first == last) {
                            throw argument_needed("--"s + opt_begin->first);
                        }
                        arg = *first;
                    }

                    option->set_arg(arg);
                }
                else {
                    if (!arg.empty()) {
                        throw argument_not_needed("--"s + opt_begin->first);
                    }

                    option->set();
                }
            }
            else {
                str.remove_prefix(1);
                while (!str.empty()) {
                    auto flag = str.front();

                    auto found = _short.find(flag);
                    if (found == _short.end()) {
                        throw option_not_found('-' + std::string{flag});
                    }

                    const auto &option = found->second;

                    if (option->needs_arg()) {
                        auto arg = std::string{str.substr(1)};
                        if (arg.empty()) {
                            if (++first == last) {
                                throw argument_needed(
                                    '-' + std::string{found->first});
                            }
                            arg = *first;
                        }
                        option->set_arg(arg);
                        break;
                    }
                    else {
                        option->set();
                    }

                    str.remove_prefix(1);
                }
            }

            ++first;
        }

        return first;
    }
    catch (const usage_error &ex) {
        std::cerr << "error: " << ex.what() << "\n\n";
        usage();
        exit(1);
    }

    template <class StringLike>
    static auto
    wrap(StringLike &&s, typename std::decay<StringLike>::type::size_type w,
         typename std::decay<StringLike>::type::size_type i = 0) {
        if (s.size() <= w) return std::forward<StringLike>(s);

        auto pos = s.rfind(' ', w);
        if (pos == s.npos) return s;

        auto end = pos + 1;
        while (s[pos - 1] == ' ') --pos;

        auto result = std::string(s, 0, pos) + "\n" + std::string(i, ' ');
        return result + wrap(std::forward<StringLike>(s).substr(end), w, i);
    }

    void usage(std::size_t screen_width = 72) const {
        if (!_synopsis.empty()) {
            std::cerr << wrap("usage: " + _synopsis, screen_width, 7)
                      << "\n\n";
        }

        auto width =
            std::ranges::max(_usage | std::views::elements<0> |
                             std::views::transform(&std::string::size));

        static constexpr std::string::size_type indent = 4;
        static constexpr std::string::size_type padding = 8;

        width += indent + padding;

        for (const auto &[flags, text] : _usage) {
            std::string description(indent, ' ');

            description.append(flags);
            description.resize(width, ' ');
            description.append(text);

            std::cerr << wrap(description, screen_width, width)
                      << std::endl;
        }
    }
};

} // namespace cli

#endif
