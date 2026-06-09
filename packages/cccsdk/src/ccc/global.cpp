#include "ccc/global.h"

#include "ccc/cmd/build.hpp"
#include "ccc/cmd/clean.hpp"
#include "ccc/cmd/desc.hpp"
#include "ccc/cmd/help.hpp"
#include "ccc/command.h"
#include "ccc/info.hpp"
#include "ccc/project.h"
#include "util/io.h"

#include <cstddef>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string,
                   std::vector<std::pair<std::string, std::source_location>>>
    ccc::global_var::descs;

std::vector<ccc::third_party*> ccc::global_var::projects;

std::unordered_map<std::string, std::unordered_map<ccc::command::priority,
                                                   std::vector<ccc::command*>>>
    ccc::global_var::cmds;
/* Built-in commands. */
static ccc::command build_cmd("build", build_func,
                              "Build the projects based on project.cpp.",
                              ccc::command::priority::high);
static ccc::command desc_cmd("desc", desc_func,
                             "Get a description of what you want to know.",
                             ccc::command::priority::high);
static ccc::command clean_cmd("clean", clean_func,
                              "Remove products from the projects.",
                              ccc::command::priority::high);
static ccc::command version_cmd(
    "version",
    [](std::vector<std::string>) {
        ccc::io::println("ccc version: " + ccc::info::version);
    },
    "Print the version of ccc.", ccc::command::priority::low);
static ccc::command help_cmd("help", help_func,
                             "Print the help message about ccc.",
                             ccc::command::priority::low);
static ccc::command project_cmd(
    "project", [](std::vector<std::string>) {},
    "Only generate the executable project file without performing any "
    "other operations.",
    ccc::command::priority::high);
void ccc::global_var::add_desc(const std::string name, const std::string desc,
                               std::source_location loc) {
    descs[name].push_back({desc, loc});
}

std::vector<std::pair<std::string, std::source_location>>
ccc::global_var::get_descs(const std::string name) {
    return descs[name];
}

void ccc::global_var::add_cmd(const std::string name, const ccc::command* cmd,
                              ccc::command::priority prio) {
    cmds[name][prio].push_back(const_cast<ccc::command*>(cmd));
}

const ccc::command* ccc::global_var::get_cmd(const std::string name) {
    // Check if the command exists
    if (cmds.find(name) == cmds.end()) {
        ccc::io::error("The command '" + name + "' is not registered.");
        exit(-1);
    }

    // Get priority map
    auto& priority_map = cmds[name];

    // Find the highest priority (high > normal > low)
    auto max_prio = ccc::command::priority::low;
    for (const auto& [prio, cmds] : priority_map) {
        if (!cmds.empty() && prio > max_prio) {
            max_prio = prio;
        }
    }

    // Get the command list for the highest priority
    auto& candidates = priority_map.at(max_prio);

    // Check if there are multiple candidates
    if (candidates.size() != 1) {
        std::string msg = "The multiple commands are registered for '" + name +
                          "' with the same and highest priority(" + max_prio +
                          ").\n";

        // Add the position information of the candidates to the message
        for (size_t i = 0; i < candidates.size(); i++) {
            msg += std::string("[") + std::to_string(i) + "] " +
                   std::string(candidates[i]->loc.file_name()) + ":" +
                   std::to_string(candidates[i]->loc.line());
            if (i != candidates.size() - 1) {
                msg += "\n";
            }
        }

        ccc::io::error(msg);
        exit(-1);
    }

    // Return the single command
    return candidates.front();
}

void ccc::global_var::add_project(ccc::third_party* project) {
    projects.push_back(project);
}

const std::vector<ccc::third_party*>& ccc::global_var::get_projects() {
    return ccc::global_var::projects;
}