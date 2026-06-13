#include "ccc/execution.h"

#include "util/file.hpp"
#include "util/io.h"
#include <string>

ccc::execution::execution(std::string name, std::string description,
                          std::source_location loc)
    : ccc::compile_task(name, description, loc) {
    output_path = "./build/bin";
}

void ccc::execution::init(const ccc::config& project_cfg) {
    // Initialize task
    this->config.toolchain =
        // If the toolchain of the execution is not empty, use it.
        !this->config.toolchain.is_empty() ? this->config.toolchain

        // If the toolchain of the project is not empty, use it.
        : !project_cfg.toolchain.is_empty()
            ? project_cfg.toolchain

            // Use the built-in toolchain.
            : ccc::built_in_toolchain::gnu_toolchain();

    // Set the compile foramt and the link format
    this->config.toolchain.compile_format =
        this->config.toolchain.execution_compile_format;
    this->config.toolchain.link_format =
        this->config.toolchain.execution_link_format;

    // Add the suffix '.exe' when the target os is windows.
    if (this->config.toolchain.target_os == system_type::windows_os &&
        this->name.find(".exe") == std::string::npos) {
        this->name += ".exe";
    }
}

void ccc::execution::link(const ccc::config& project_cfg) {
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
             {(this->output_path.empty() ? "./build/bin" : this->output_path) +
              "/" + this->name}},
            {"LIBRARY_FILES", {this->lib_files.begin(), this->lib_files.end()}},
            {"LIBRARY_FOLDERS",
             {this->config.library_folder_paths.begin(),
              this->config.library_folder_paths.end()}},
            {"LINK_FLAGS",
             {this->config.link_flags.begin(), this->config.link_flags.end()}}};

    std::string cmd = this->config.toolchain.link_format.replace(replacements);

    // Link
    if (!ccc::io::exec_command(cmd,
                               project_cfg.is_print && this->config.is_print,
                               project_cfg.is_print && this->config.is_print)) {
        this->status.push_back("Fail to link execution: " + this->output_path +
                               "/" + this->name);
    }
}

void ccc::execution::transmit(ccc::compile_task& super) {
    if (super.name.length())
        return;
}
