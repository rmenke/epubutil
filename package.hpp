#ifndef _package_hpp_
#define _package_hpp_

#include <filesystem>

namespace epub {

class package {
public:
    void write(const std::filesystem::path &path) const;
};

}

#endif
