#ifndef _metadata_hpp_
#define _metadata_hpp_

#include <charconv>
#include <chrono>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace epub {

/// @brief Generate a UUID.
///
/// Generates a random (version 4) UUID for use as a publication
/// identifier.
///
/// @returns a string containing the UUID as a hex string with parts
/// separated by hyphens
///
extern std::u8string generate_uuid();

/// @brief Generate a XML ID.
///
/// Generates a sequential identifier that can be used as an XML ID.
///
/// @returns a string
///
extern std::u8string generate_id();

/// @brief An individual or organizational creator.
///
/// The creator is the primary source for the content: the person
/// responsible for its creation.  It is usually a name in display
/// form.  There are two refinements that can be added to a creator.
/// The @em file-as refinement is a string that is appropriate for
/// sorting, such as "last-name, first-name."  The @em role refinement
/// is the MARC relator code describing how the creator was involved:
/// "aut" for author, "ill" for illustrator, and so on.
///
/// @sa <a href="https://www.loc.gov/marc/relators/relaterm.html">MARC
/// source codes</a> at the Library of Congress
///
class creator {
    std::u8string _name;    ///< The creator's name.
    std::u8string _file_as; ///< The string used to sort and index.
    std::u8string _role;    ///< The role the creator played.

  public:
    /// Create a creator instance for the person or organization so
    /// named.
    ///
    /// This is both a constructor and a conversion operator.  If the
    /// programmer is not interested in refinements, they can use the
    /// @c creator values as normal UTF-8 strings.
    ///
    /// @param name the name of the person or organization
    creator(std::u8string name)
        : _name(std::move(name)) {}

    /// Convert the creator object to a UTF-8 string automatically.
    ///
    /// @return the name of the creator
    operator const std::u8string &() const {
        return _name;
    }

    /// Retrieve the "file-as" refinement.
    ///
    /// @returns the "file-as" refinement string
    const std::u8string &file_as() const {
        return _file_as;
    }

    /// Set the "file-as" refinement.
    ///
    /// This is the string used for sorting and indexing.
    /// Traditionally it is "last, first" but may be in different
    /// forms if presentation and textual representation are
    /// different.
    ///
    /// @param file_as the "file-as" refinement string
    void file_as(auto &&file_as) {
        _file_as = std::forward<decltype(file_as)>(file_as);
    }

    /// Retrieve the "role" refinement.
    ///
    /// @returns a three-letter UTF-8 string or the empty string if no
    /// code has been assigned
    const std::u8string &role() const {
        return _role;
    }

    /// Set the "role" refinement.
    ///
    /// This is a three letter MARC code describing how the creator
    /// was involved in the production of the document.  Valid codes
    /// are listed above.
    ///
    /// @param role a three-letter UTF-8 string or the empty string to
    /// unset
    void role(auto &&role) {
        _role = std::forward<decltype(role)>(role);
    }
};

/// @brief An indication that the publication is part of a larger set.
///
/// Adding a collection tag indicates that the publication is either a
/// single volume of a larger set or a single issue of an ongoing
/// series.
///
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

    enum type type() const {
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

/// @brief Rendering orientation.
///
enum class orientation {
    automatic, ///< No constraint on orientation.
    landscape, ///< Render in landscape orientation.
    portrait,  ///< Render in portrait orientation.
};

/// @brief The metadata component of the package document.
///
/// This class covers all the information that describes the
/// publication as a whole: the publication identifier, the
/// publication title, and the language in which the content documents
/// are written.
///
/// Other metadata, such as the author, is optional.
///
class metadata {
    std::u8string _identifier;
    std::u8string _title;
    std::u8string _language;
    std::u8string _description;

    std::vector<creator> _creators;
    std::vector<collection> _collections;

    bool _pre_paginated = false;
    orientation _orientation = orientation::automatic;

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

    const std::u8string description() const {
        return _description;
    }
    void description(std::u8string description) {
        _description = std::move(description);
    }

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
    /// document is assumed to be a single page.  The content
    /// documents must have a viewport defined for this to work.
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
        static constexpr std::u8string_view pre_paginated =
            u8"pre-paginated";
        static constexpr std::u8string_view reflowable = u8"reflowable";
        return std::u8string(_pre_paginated ? pre_paginated : reflowable);
    }

    /// @brief Select the rendition orientation.
    ///
    /// Calling this will tell the reading system to render the
    /// content in landscape orientation.
    ///
    void landscape() {
        orientation(orientation::landscape);
    }

    /// @brief Select the rendition orientation.
    ///
    /// Calling this will tell the reading system to render the
    /// content in portrait orientation.
    ///
    void portrait() {
        orientation(orientation::portrait);
    }

    /// @brief Select the rendition orientation.
    ///
    /// @param o the orientation to use
    /// @sa epub::orientation
    void orientation(enum orientation o) {
        _orientation = o;
    }

    /// @brief The rendition orientation type.
    ///
    /// @return a string with the appropriate value for the @c
    /// rendition:orientation property
    ///
    std::u8string orientation() const {
        switch (_orientation) {
            case orientation::landscape:
                return u8"landscape";
            case orientation::portrait:
                return u8"portrait";
            default:
                return u8"auto";
        }
    }
};

} // namespace epub

#endif
