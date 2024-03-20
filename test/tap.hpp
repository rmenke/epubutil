// Copyright (C) 2020-2024 by Rob Menke.  All rights reserved.  See
// accompanying LICENSE.txt file for details.

#ifndef _tap_hpp_
#define _tap_hpp_

#include <cxxabi.h>

#include <cmath>
#include <cstdio>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <new>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace tap {

// Overload this for a custom value display (or overload the iostream
// operator <<() instead).  All TAP functions use this for output.

template <class Arg>
inline void sprint_one(std::ostream &os, const Arg &arg) {
    os << arg;
}

template <typename T>
struct _free_deleter {
    void operator()(T *ptr) const {
        free(ptr);
    }
};

template <typename T>
using malloc_ptr = std::unique_ptr<T, _free_deleter<T>>;

inline std::string demangle(const std::string &mangled) {
    int status = 0;

    malloc_ptr<char> result{__cxxabiv1::__cxa_demangle(
        mangled.c_str(), nullptr, nullptr, &status)};

    if (!result) {
        switch (status) {
            case -1:
                throw std::bad_alloc{};
            case -2:
                throw std::invalid_argument{"not a mangled name"};
            case -3:
                throw std::invalid_argument{"invalid argument"};
            default:
                throw std::runtime_error{"__cxa_demangle: status = " +
                                         std::to_string(status)};
        }
    }

    return std::string{result.get()};
}

inline void sprint_one(std::ostream &os, const std::type_info &ti) {
    os << demangle(ti.name());
}

inline void sprint_one(std::ostream &os, std::exception_ptr ptr) {
    try {
        std::rethrow_exception(ptr);
    }
    catch (const std::exception &ex) {
        os << "exception " << demangle(typeid(ex).name()) << ": "
           << ex.what();
    }
    catch (...) {
        auto ti = __cxxabiv1::__cxa_current_exception_type();
        os << "exception " << demangle(ti->name());
    }
}

static inline void sprint(std::ostream &) {}

template <class Arg, class... Args>
static inline void sprint(std::ostream &os, Arg &&arg, Args &&...args) {
    sprint_one(os, std::forward<Arg>(arg));
    sprint(os, std::forward<Args>(args)...);
}

template <class... Args>
static inline void print(Args &&...args) {
    sprint(std::cout, std::forward<Args>(args)...);
}

template <class... Args>
static inline void println(Args &&...args) {
    print(std::forward<Args>(args)..., '\n');
}

template <class... Args>
static inline void diag(Args &&...args) {
    println("# ", std::forward<Args>(args)...);
}

struct skip_all_t {};

static constexpr skip_all_t skip_all{};

class test_plan {
    unsigned _plan;
    unsigned _done = 0;
    bool _failed = false;

    test_plan *_parent;

    static inline test_plan *_current_plan = nullptr; // NOLINT

    std::string _todo;

  public:
    test_plan(unsigned plan = 0)
        : _plan(plan)
        , _parent(_current_plan) {
        _current_plan = this;

        if (_parent) {
            if (_plan > (_parent->_plan - _parent->_done)) {
                throw std::logic_error{"too many subtests"};
            }
        }

        if (plan) println("1..", plan);
    }

    template <class... Args>
    test_plan(skip_all_t, unsigned plan, Args &&...args)
        : _plan(plan)
        , _done(plan)
        , _parent(_current_plan) {
        _current_plan = this;

        if (!_parent) {
            println("1..0 # SKIP ", std::forward<Args>(args)...);
            std::exit(0);
        }

        for (unsigned i = 1; i <= _plan; ++i) {
            _parent->skip(args...);
        }
    }

    test_plan(const test_plan &) = delete;

    test_plan(test_plan &&r)
        : _plan(std::exchange(r._plan, 0))
        , _done(std::exchange(r._done, 0))
        , _parent(std::exchange(r._parent, nullptr)) {
        _current_plan = this;
    }

    test_plan &operator=(const test_plan &) = delete;
    test_plan &operator=(test_plan &&) = delete;

    ~test_plan() noexcept(false) {
        if (_current_plan != this) {
            throw std::logic_error{"incorrect nesting of plans"};
        }

        _current_plan = _parent;

        if (_plan == 0 && !_parent) {
            println("1..", _done);
        }
        else if (_plan != _done) {
            diag("Looks like you planned ", _plan, ' ',
                 (_plan == 1 ? "test" : "tests"), " but ran ", _done);
        }
    }

    static test_plan *current_plan() {
        auto plan = _current_plan;
        if (!plan) throw std::logic_error{"no plan"};
        return plan;
    }

    int status() const {
        return _failed ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    template <class... Args>
    void pass(Args &&...args) {
        ++_done;

        if (_parent) return _parent->pass(std::forward<Args>(args)...);

        print("ok ", _done);

        if (!_todo.empty()) {
            print(" # TODO ", _todo);
        }
        else if constexpr (sizeof...(args)) {
            print(" - ", std::forward<Args>(args)...);
        }

        println();
    }

    template <class... Args>
    void fail(Args &&...args) {
        ++_done;

        if (_parent) return _parent->fail(std::forward<Args>(args)...);

        print("not ok ", _done);

        if (!_todo.empty()) {
            print(" # TODO ", _todo);
        }
        else if constexpr (sizeof...(args)) {
            print(" - ", std::forward<Args>(args)...);
        }

        println();

        _failed = true;
    }

    template <class... Args>
    void skip(Args &&...args) {
        if (_todo.empty()) {
            println("ok ", ++_done, " # SKIP ",
                    std::forward<Args>(args)...);
        }
        else {
            println("not ok ", ++_done, " # TODO & SKIP ", _todo);
        }
    }

    template <class... Args>
    std::string todo(Args &&...args) {
        std::ostringstream os;
        sprint(os, std::forward<Args>(args)...);
        return std::exchange(_todo, std::move(os).str());
    }
};

template <class... Args>
static inline void pass(Args &&...args) {
    test_plan::current_plan()->pass(std::forward<Args>(args)...);
}

template <class... Args>
static inline void fail(Args &&...args) {
    test_plan::current_plan()->fail(std::forward<Args>(args)...);
}

template <class... Args>
static inline bool ok(bool result, Args &&...args) {
    auto plan = test_plan::current_plan();

    if (result)
        plan->pass(std::forward<Args>(args)...);
    else
        plan->fail(std::forward<Args>(args)...);

    return result;
}

template <template <class> class T>
static inline const char *const cmp_op = nullptr;

template <>
inline const char *const cmp_op<std::equal_to> = "==";
template <>
inline const char *const cmp_op<std::not_equal_to> = "!=";
template <>
inline const char *const cmp_op<std::greater> = ">";
template <>
inline const char *const cmp_op<std::greater_equal> = ">=";
template <>
inline const char *const cmp_op<std::less> = "<";
template <>
inline const char *const cmp_op<std::less_equal> = "<=";

template <template <class> class Comp, class X, class Y, class... Args>
static inline bool cmp(X &&x, Y &&y, Args &&...args) {
    if (!ok(Comp<void>{}(x, y), std::forward<Args>(args)...)) {
        diag(x, ' ', cmp_op<Comp>, ' ', y, ": failed");
        return false;
    }
    return true;
}

template <class X, class Y, class... Args>
static inline bool eq(X &&x, Y &&y, Args &&...args) {
    return cmp<std::equal_to>(std::forward<X>(x), std::forward<Y>(y),
                              std::forward<Args>(args)...);
}

template <class X, class Y, class... Args>
static inline bool ne(X &&x, Y &&y, Args &&...args) {
    return cmp<std::not_equal_to>(std::forward<X>(x), std::forward<Y>(y),
                                  std::forward<Args>(args)...);
}

template <class X, class Y, class... Args>
static inline bool gt(X &&x, Y &&y, Args &&...args) {
    return cmp<std::greater>(std::forward<X>(x), std::forward<Y>(y),
                             std::forward<Args>(args)...);
}

template <class X, class Y, class... Args>
static inline bool ge(X &&x, Y &&y, Args &&...args) {
    return cmp<std::greater_equal>(std::forward<X>(x), std::forward<Y>(y),
                                   std::forward<Args>(args)...);
}

template <class X, class Y, class... Args>
static inline bool lt(X &&x, Y &&y, Args &&...args) {
    return cmp<std::less>(std::forward<X>(x), std::forward<Y>(y),
                          std::forward<Args>(args)...);
}

template <class X, class Y, class... Args>
static inline bool le(X &&x, Y &&y, Args &&...args) {
    return cmp<std::less_equal>(std::forward<X>(x), std::forward<Y>(y),
                                std::forward<Args>(args)...);
}

template <class X, class Y, class Z, class... Args>
static inline bool within(X &&x, Y &&y, Z &&z, Args &&...args) {
    if (!ok(std::fabs(x - y) <= z, std::forward<Args>(args)...)) {
        diag(y, " = ", x, "±", z, ": failed");
        return false;
    }
    return true;
}

template <class Str, class RE, class... Args>
static inline bool like(Str &&str, RE &&re, Args &&...args) {
    using CharT = typename std::remove_reference<Str>::type::value_type;

    if (!ok(std::regex_match(str, std::basic_regex<CharT>{re}),
            std::forward<Args>(args)...)) {
        diag(str, " is like ", re, ": failed");
        return false;
    }
    return true;
}

template <class T, class P, class... Args>
static inline bool is_instanceof(P &&p, Args &&...args) {
    std::type_info const &expected = typeid(T);
    std::type_info const &actual = typeid(p);

    if (!ok(expected == actual, std::forward<Args>(args)...)) {
        diag("type of parameter is ", actual.name());
        diag("     but expected    ", expected.name());
        return false;
    }
    return true;
}

template <class... Args>
static inline test_plan skip(unsigned count, Args &&...args) {
    return test_plan{skip_all, count, args...};
}

class todo {
    test_plan *_plan;
    std::string _old_todo;

  public:
    template <class... Args>
    todo(Args &&...args)
        : _plan(test_plan::current_plan())
        , _old_todo(_plan->todo(std::forward<Args>(args)...)) {}
    todo(const todo &) = delete;
    todo(todo &&) = delete;

    todo &operator=(const todo &) = delete;
    todo &operator=(todo &&) = delete;

    ~todo() {
        _plan->todo(_old_todo);
    }
};

#define TODO(REASON) if (tap::todo __todo{REASON}; true)

template <class... Args>
[[noreturn]] static inline void bail_out(Args &&...args) {
    println("Bail out! ", std::forward<Args>(args)...);
    exit(1);
}

template <class... Args>
std::exception_ptr catch_exception(Args &&...args) {
    std::invoke(std::forward<Args>(args)...);
    return nullptr;
}

template <class Exception, class... Exceptions, class... Args>
std::exception_ptr catch_exception(Args &&...args) {
    try {
        return catch_exception<Exceptions...>(std::forward<Args>(args)...);
    }
    catch (const Exception &ex) {
        return std::current_exception();
    }
}

template <class... Exception, class... Args>
auto expected_exception(Args &&...args) {
    auto ex = catch_exception<Exception...>(std::forward<Args>(args)...);

    if (ex) {
        pass("caught ", ex);
    }
    else {
        fail("exception expected but not thrown");
    }

    return ex;
}

} // namespace tap

#endif
