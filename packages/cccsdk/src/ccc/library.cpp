#include "ccc/library.h"

#include "util/file.hpp"
#include "util/io.h"
#include <string>

ccc::library::library(std::string name, ccc::library_type type,
                      std::string description, std::source_location loc)
    : ccc::compile_task(name, description, loc), type(type) {
    if (name.find(".") != std::string::npos ||
        name.find("/") != std::string::npos ||
        name.find("\\") != std::string::npos) {
        ccc::io::println("ccc: \033[31merror:\033[0m " +
                         std::string("The name of library") + "(" + name + " " +
                         loc.file_name() + ":" + std::to_string(loc.line()) +
                         ") cannot contain special characters['.' '/' '\\'].");
        exit(1);
    }

    output_path = "./build/lib";
}

void ccc::library::init(const ccc::config& project_cfg) {
    // Initialize task
    this->config.toolchain =
        // If the toolchain of the library is not empty, use it.
        !this->config.toolchain.is_empty() ? this->config.toolchain

        // If the toolchain of the project is not empty, use it.
        : !project_cfg.toolchain.is_empty()
            ? project_cfg.toolchain

            // Use the built-in toolchain.
            : ccc::built_in_toolchain::gnu_toolchain();

    // Set the compile foramt and the link format
    if (this->type == library_type::static_library) {
        this->config.toolchain.compile_format =
            this->config.toolchain.static_library_compile_format;
        this->config.toolchain.link_format =
            this->config.toolchain.static_library_link_format;
    } else if (this->type == library_type::shared_library) {
        this->config.toolchain.compile_format =
            this->config.toolchain.shared_library_compile_format;
        this->config.toolchain.link_format =
            this->config.toolchain.shared_library_link_format;
    }

    // Set the library name prefix and suffix according to the target
    // operating system. Windows
    if (this->name.find(".") == std::string::npos) {
        if (this->config.toolchain.target_os == ccc::system_type::windows_os) {
            if (this->type == library_type::static_library) {
                this->name = "lib" + this->name + ".lib";
            } else if (this->type == library_type::shared_library) {
                this->name = this->name + ".dll";
            }
        }
        // Linux
        else if (this->config.toolchain.target_os ==
                 ccc::system_type::linux_os) {
            if (this->type == library_type::static_library) {
                this->name = "lib" + this->name + ".a";
            } else if (this->type == library_type::shared_library) {
                this->name = "lib" + this->name + ".so";
            }
        }
    }
}

void ccc::library::link(const ccc::config& project_cfg) {
    // Merge the project's link flags with the library's link flags.
    this->config.link_flags.insert(this->config.link_flags.begin(),
                                   project_cfg.link_flags.begin(),
                                   project_cfg.link_flags.end());

    // If the output_path doesn't exist, create it.
    std::string target_folder =
        extractPath(this->output_path + "/" + this->name);
    if (!(fs::exists(target_folder) && fs::is_directory(target_folder))) {
        fs::create_directories(target_folder);
    }

    auto replacements =
        std::unordered_map<std::string, std::vector<std::string>>{
            {"LINKER",
             {!this->config.toolchain.linker.empty()
                  ? this->config.toolchain.linker
              : !project_cfg.toolchain.linker.empty()
                  ? project_cfg.toolchain.linker
                  : "g++"}},
            {"OBJECT_FILES", {this->obj_files.begin(), this->obj_files.end()}},
            {"OUTPUT_FILE",
             {(this->output_path.empty() ? "./build/lib" : this->output_path) +
              "/" + this->name}},
            {"LIBRARY_FILES", {this->lib_files.begin(), this->lib_files.end()}},
            {"LIBRARY_FOLDERS",
             {this->config.library_folder_paths.begin(),
              this->config.library_folder_paths.end()}},
            {"LINK_FLAGS",
             {this->config.link_flags.begin(), this->config.link_flags.end()}}};

    std::string cmd = this->config.toolchain.link_format.replace(replacements);
    // Link
    ccc::io::exec_command(cmd, project_cfg.is_print && this->config.is_print,
                          project_cfg.is_print && this->config.is_print);
}

void ccc::library::transmit(ccc::compile_task& super) {
    // For static libraries
    if (this->type == static_library) {
        // Determine whether to add the prefix and the suffix.
        if (this->name.find(".") == std::string::npos)
            this->init(super.config);
        super.obj_files.push_back(this->output_path + "/" + this->name);
    }

    // For dynamic libraries
    else if (this->type == shared_library) {
        // Remove the prefix and suffix of the library name.
        std::string lib_name = this->name;
        if (lib_name.size() >= 4 &&
            lib_name.substr(lib_name.size() - 4) == ".dll") {
            lib_name = lib_name.substr(0, lib_name.size() - 4);
        }
        if (lib_name.size() >= 3 &&
            lib_name.substr(lib_name.size() - 3) == ".so") {
            lib_name = lib_name.substr(0, lib_name.size() - 3);
        }
        if (lib_name.size() >= 3 && lib_name.substr(0, 3) == "lib") {
            lib_name = lib_name.substr(3);
        }

        super.config.library_folder_paths.push_back(this->output_path);
        super.lib_files.push_back(lib_name);
    }
}