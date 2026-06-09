#ifndef __CCC_CMD_CLEAN_HPP__
#define __CCC_CMD_CLEAN_HPP__

#include "ccc/compile_task.h"
#include "ccc/global.h"
#include "util/io.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

inline auto clean_func = [](std::vector<std::string> args) {
    // Check if the project.cpp exists.
    if (!std::filesystem::exists("project.cpp") ||
        !std::filesystem::is_regular_file("project.cpp")) {
        ccc::io::error("The command 'clean' cannot run in a "
                       "directory without project.cpp.");
        exit(-1);
    }
    /* Traverse all projects. */
    auto projects = ccc::global_var::get_projects();
    uint64_t counter = 0;
    for (auto project_base : projects) {

        /* clean the ccc-project */
        if (typeid(*project_base) == typeid(ccc::project)) {
            auto project = dynamic_cast<ccc::project*>(project_base);
            project->init_func(project, "clean", args);

            /* Traverse all tasks.(Delete the folders corresponding to output
             * path and obj path.) */
            for (size_t i = 0; i < project->tasks.size(); i++) {
                project->tasks[i]->init(project->config);
                if (std::filesystem::exists(project->tasks[i]->output_path +
                                            "/" + project->tasks[i]->name)) {
                    ccc::io::println("[" + std::to_string(counter++) +
                                     "] clean " +
                                     project->tasks[i]->output_path + "/" +
                                     project->tasks[i]->name + " file from " +
                                     project->loc_info + ": => " +
                                     project->tasks[i]->loc_info);

                    std::filesystem::remove(project->tasks[i]->output_path +
                                            "/" + project->tasks[i]->name);
                }
            }

            project->exit_func(project, "clean", args);

            if (project != projects.back())
                ccc::io::println("");
        }
    }
};

#endif