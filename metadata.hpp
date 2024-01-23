#ifndef _metadata_hpp_
#define _metadata_hpp_

#include <charconv>
#include <chrono>
#include <stdexcept>
#include <string>
#include <system_error>

namespace epub {

extern std::u8string generate_id();
extern std::u8string generate_uuid();

class creator {
    std::u8string _creator;
    std::u8string _file_as;
    std::u8string _role;

  public:
    creator(std::u8string creator)
        : _creator(std::move(creator)) {}

    creator(const creator &) = default;
    creator(creator &&) = default;

    ~creator() = default;

    creator &operator=(const creator &) = default;
    creator &operator=(creator &&) = default;

    operator const std::u8string &() const {
        return _creator;
    }

    const std::u8string &file_as() const {
        return _file_as;
    }
    void file_as(auto &&file_as) {
        _file_as = std::forward<decltype(file_as)>(file_as);
    }

    const std::u8string &role() const {
        return _role;
    }
    void role(auto &&role) {
        _role = std::forward<decltype(role)>(role);
    }
};

class collection {
  public:
    enum class type { unspecified, series, set };

  private:
    std::u8string _name;
    type _type = type::unspecified;
    std::u8string _group_position;

  public:
    collection(std::u8string name)
        : _name(std::move(name)) {}

    collection(const collection &) = default;
    collection(collection &&) = default;

    ~collection() = default;

    collection &operator=(const collection &) = default;
    collection &operator=(collection &&) = default;

    operator const std::u8string &() const {
        return _name;
    }

    enum type type()const {
        return _type;
    }
    void type(enum type type) {
        _type = type;
    }

    std::u8string group_position() const {
        return _group_position;
    }
    void group_position(unsigned position) {
        char buffer[128];
        auto result = std::to_chars(buffer, buffer + 128, position);
        if (result.ec != std::errc{}) {
            throw std::system_error(std::make_error_code(result.ec));
        }
        _group_position.assign(buffer, result.ptr);
    }
    void group_position(std::u8string position) {
        _group_position = std::move(position);
    }
};

class metadata {
    std::u8string _identifier;
    std::u8string _title;
    std::u8string _language;

    std::vector<creator> _creators;
    std::vector<collection> _collections;

    bool _pre_paginated = false;

  public:
    metadata()
        : _identifier(std::u8string{u8"urn:uuid:"} + generate_uuid())
        , _title(u8"Untitled")
        , _language(u8"en-US") {}

    /// @name Dublin Core required elements

    /// @{

    const std::u8string identifier() const {
        return _identifier;
    }
    void identifier(std::u8string identifier) {
        _identifier = std::move(identifier);
    }
    const std::u8string title() const {
        return _title;
    }
    void title(std::u8string title) {
        _title = std::move(title);
    }
    const std::u8string language() const {
        return _language;
    }

    /// @}

    /// @name Dublin Core optional elements

    /// @{

    const auto &creators() const {
        return _creators;
    }
    auto &creators() {
        return _creators;
    }

    const auto &collections() const {
        return _collections;
    }
    auto &collections() {
        return _collections;
    }

    /// @}

    /// @brief Select the rendition layout controlling reflow.
    ///
    /// Calling this disables reflow in the document.  Each content
    /// document is assumed to be a single page.
    ///
    void pre_paginated() {
        _pre_paginated = true;
    }

    /// @brief Select the rendition layout controlling reflow.
    ///
    /// Calling this enables reflow in the document.  The content
    /// documents will be separated into individual pages according to
    /// the user's display options.
    ///
    void reflow() {
        _pre_paginated = false;
    }

    /// @brief The rendition layout type.
    ///
    /// @return a string with the appropriate value for the @c
    /// rendition:layout property
    ///
    std::u8string layout() const {
        return _pre_paginated ? u8"pre-paginated" : u8"reflowable";
    }
};

} // namespace epub

#endif
