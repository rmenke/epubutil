#ifndef _xml_hpp_
#define _xml_hpp_

#include <filesystem>

namespace epub {

class package;

}

namespace epub::xml {

extern void write_package(const std::filesystem::path &path,
                          const class package &package);

}

#endif
