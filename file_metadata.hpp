#ifndef _file_metadata_hpp_
#define _file_metadata_hpp_

#include <map>
#include <optional>
#include <string>
#include "src/compat.hpp"

namespace epub {

/// @brief A class for managing metadata key-value pairs.
///
/// This specialization of @c std::map adds methods for looking up
/// values in @c const maps without throwing exceptions when the keys
/// are absent.
///
struct file_metadata : std::map<std::u8string, std::u8string> {
    using std::map<key_type, mapped_type>::map;

    /// @brief Access a specified element.
    ///
    /// If the element cannot be found, returns an empty optional.
    ///
    /// @param key the key of the element to find
    /// @returns an optional @c mapped_type set to the mapped value if
    ///   found, and empty otherwise
    ///
    std::optional<mapped_type> get(const key_type &key) const noexcept {
        if (auto found = find(key); found != end()) {
            return found->second;
        }
        return std::nullopt;
    }

    /// @brief Access a specified element.
    ///
    /// If the element cannot be found, returns a @c mapped_type
    /// object converted from the @p default_value argument.
    ///
    /// @param key the key of the element to find
    /// @param default_value the value to return if the key is not found
    /// @returns either the mapped value or @c default_value converted
    ///   to @c mapped_type
    ///
    template <class U>
    mapped_type get(const key_type &key, U &&default_value) const
        noexcept(std::is_nothrow_convertible<U, mapped_type>::value) {
        return get(key).value_or(std::forward<U>(default_value));
    }
};

} // namespace epub

#endif
