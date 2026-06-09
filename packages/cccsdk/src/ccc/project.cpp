#include "ccc/project.h"

#include "ccc/global.h"
#include "util/io.h"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>

ccc::third_party::third_party(std::string name, std::string description,
                              bool ccc_manage, std::string path,
                              std::source_location loc)
    : name(name), description(description), path(path), loc(loc) {
    // Add description
    ccc::global_var::add_desc(name, description, loc);

    // Get the simple information of the location.
    loc_info = "[" + this->name + " " + std::string(loc.file_name()) + ":" +
               std::to_string(loc.line()) + "]";

    // Add project
    if (ccc_manage)
        ccc::global_var::add_project(this);
}

ccc::project::project(
    std::string name,
    auto (*init_func)(project*, std::string, std::vector<std::string>)->void,
    auto (*exit_func)(project*, std::string, std::vector<std::string>)->void,
    std::string description, std::string path, std::source_location loc)
    : third_party(name, description, true, path, loc), init_func(init_func),
      exit_func(exit_func) {}

bool ccc::project::process(std::vector<std::string> args) {
    auto process_tasks = [this]() {
        bool status = true;

        for (size_t i = 0; i < tasks.size(); i++) {
            std::vector<std::string> path{this->loc_info + ":"};
            tasks[i]->process(config, path);
            if (tasks[i]->status.size() != 0) {
                status = false;
            }

            if (i != tasks.size() - 1)
                ccc::io::println("");
        }

        return status;
    };

    bool status = true;
    this->init_func(this, "build", args);

    // process the project.
    if (!process_tasks()) {
        status = false;
    }

    this->exit_func(this, "build", args);
    return status;
}

void ccc::project::add_task(ccc::compile_task* task) { tasks.push_back(task); }