#ifndef __CCC_INFO_HPP__
#define __CCC_INFO_HPP__

#include <string>

namespace ccc {
namespace info {
inline std::string help_msg =
    "Usage: ccc <command> [arguments]...\n"
    "Built-in commands:\n"
    "    project              Only generate/update the executable project file "
    "without performing any other operations.(You can use this command with "
    "the 'flags=\"-g\"' argument to generate the executable project file that "
    "can be debugged.)\n"
    "    build                Compile the project.\n"
    "    desc                 Get a description of what you want to know.\n"
    "    clean                Clean the product during the build process.\n"
    "    version              Get the version of ccc.\n"
    "    help                 Get help information.\n";

inline std::string version = "0.0.0.20250609";

} // namespace info
} // namespace ccc

#endif
