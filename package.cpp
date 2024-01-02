#include "package.hpp"
#include <fstream>

namespace epub {

void package::write(const std::filesystem::path &path) const {
    std::ofstream{path} << R"(<package version="3.0" xmlns="http://www.idpf.org/2007/opf"/>)";
}

}
