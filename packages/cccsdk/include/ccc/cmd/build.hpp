#ifndef __CCC_CMD_BUILD_HPP__
#define __CCC_CMD_BUILD_HPP__

#include "ccc/global.h"
#include "util/io.h"

#include <filesystem>
#include <string>
#include <vector>

inline auto build_func = [](std::vector<std::string> args) {
    // Check if the project.cpp exists.
    if (!std::filesystem::exists("project.cpp") ||
        !std::filesystem::is_regular_file("project.cpp")) {
        ccc::io::error("The command 'build' cannot run in a "
                       "directory without project.cpp.");
        exit(-1);
    }

    // Process all projects.
    bool status = true;
    for (auto project : ccc::global_var::get_projects()) {
        status = !project->process(args) ? false : status;
    }
    if (!status)
        exit(-1);
};

#endif