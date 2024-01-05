#ifndef _package_hpp_
#define _package_hpp_

#include "manifest.hpp"
#include "metadata.hpp"
#include "spine.hpp"
#include "xml.hpp"

#include <filesystem>

namespace epub {

class package {
    class metadata _metadata;
    class manifest _manifest;
    class spine _spine;

  public:
    class metadata &metadata() {
        return _metadata;
    }
    const class metadata &metadata() const {
        return _metadata;
    }

    class manifest &manifest() {
        return _manifest;
    }
    const class manifest &manifest() const {
        return _manifest;
    }

    class spine &spine() {
        return _spine;
    }
    const class spine &spine() const {
        return _spine;
    }

    void write(const std::filesystem::path &path) const {
        xml::write_package(path, *this);
    }
};

} // namespace epub

#endif
