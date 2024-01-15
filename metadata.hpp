#ifndef _metadata_hpp_
#define _metadata_hpp_

#include <chrono>
#include <stdexcept>
#include <string>

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

class metadata {
    std::u8string _identifier;
    std::u8string _title;
    std::u8string _language;

    std::vector<creator> _creators;

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

    template <std::ranges::range Creators>
    void creators(const Creators &creators) {
        _creators.assign(std::ranges::begin(creators),
                         std::ranges::end(creators));
    }

    /// @}
};

} // namespace epub

#endif
