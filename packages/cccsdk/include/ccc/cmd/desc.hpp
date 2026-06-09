#ifndef __CCC_CMD_DESC_HPP__
#define __CCC_CMD_DESC_HPP__

#include "ccc/global.h"
#include "util/io.h"

#include <string>
#include <vector>

inline auto desc_func = [](std::vector<std::string> args) {
    if (args.size() != 1) {
        ccc::io::error("Usage: ccc desc <name>");
        exit(-1);
    }

    // Initialize all ccc-projects.
    for (auto project_base : ccc::global_var::get_projects()) {
        if (typeid(*project_base) == typeid(ccc::project)) {
            auto project = dynamic_cast<ccc::project*>(project_base);
            project->init_func(project, "describe", args);
        }
    }

    // Find the target description.
    auto descs = ccc::global_var::get_descs(args[0]);
    if (descs.size() != 0) {
        for (size_t i = 0; i < descs.size(); i++) {
            ccc::io::println("[" + std::to_string(i) + " " +
                             descs[i].second.file_name() + ":" +
                             std::to_string(descs[i].second.line()) +
                             "]: " + descs[i].first);
            if (i != descs.size() - 1)
                ccc::io::println("");
        }
        // Exit all ccc-projects.
        for (auto project_base : ccc::global_var::get_projects()) {
            if (typeid(*project_base) == typeid(ccc::project)) {
                auto project = dynamic_cast<ccc::project*>(project_base);
                project->exit_func(project, "describe", args);
            }
        }
        return;
    }

    // If the target description is not found
    else {
        // Exit all ccc-projects.
        for (auto project_base : ccc::global_var::get_projects()) {
            if (typeid(*project_base) == typeid(ccc::project)) {
                auto project = dynamic_cast<ccc::project*>(project_base);
                project->exit_func(project, "describe", args);
            }
        }
        ccc::io::error("Not find description for " + args[0] + ".");
        exit(-1);
    }
};

#endif