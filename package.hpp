#ifndef _package_hpp_
#define _package_hpp_

#include "metadata.hpp"
#include "xml.hpp"

#include <filesystem>

namespace epub {

class package {
    class metadata _metadata;

  public:
    class metadata &metadata() {
        return _metadata;
    }
    const class metadata &metadata() const {
        return _metadata;
    }

    void write(const std::filesystem::path &path) const {
        xml::write_package(path, *this);
    }
};

} // namespace epub

#endif
