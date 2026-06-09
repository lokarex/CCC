#ifndef __CCC_CMD_HELP_HPP__
#define __CCC_CMD_HELP_HPP__

#include "ccc/info.hpp"
#include "util/io.h"

#include <string>
#include <vector>

inline auto help_func = [](std::vector<std::string>) {
    ccc::io::print(ccc::info::help_msg);
};

#endif