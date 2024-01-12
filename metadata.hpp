#ifndef _metadata_hpp_
#define _metadata_hpp_

#include <chrono>
#include <string>

namespace epub {

extern std::u8string generate_uuid();

class metadata {
    std::u8string _identifier;
    std::u8string _title;
    std::u8string _language;

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
    /// @}
};

} // namespace epub

#endif
