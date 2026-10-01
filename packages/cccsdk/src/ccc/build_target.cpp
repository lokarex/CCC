#include "ccc/build_target.h"

#include "ccc/global.h"
#include "ccc/toolchain.h"
#include "util/io.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>

namespace fs = std::filesystem;

ccc::build_target::build_target(std::string name, std::string description,
                                std::source_location loc)
    : name(name), loc(loc) {
    // Add description
    ccc::global_var::add_desc(name, description, loc);

    this->loc_info = this->name + "(" + this->loc.file_name() + ":" +
                     std::to_string(this->loc.line()) + ")";
}
void ccc::build_target::process(const ccc::config& project_cfg,
                                std::vector<std::string>& path) {
    this->init(project_cfg);

    // Try to compile the task.
    this->compile(project_cfg, path);
    if (this->status.size() != 0) {
        // Print the error message.
        ccc::io::println(ccc::io::Red + this->loc_info +
                         ": ERROR: " + ccc::io::Reset +
                         "Pass to link because of the following reasons:");
        for (size_t i = 0; i < this->status.size(); i++) {
            ccc::io::print(ccc::io::Red + "[" + std::to_string(i) +
                           "]: " + ccc::io::Reset);
            ccc::io::println(this->status[i]);
        }

        // Remove the output file.
        fs::remove(this->output_path / this->name);
        return;
    }

    // Try to link the task.
    this->link(project_cfg);
    if (this->status.size() != 0) {
        // Print the error message.
        ccc::io::error(ccc::io::Red + this->loc_info +
                       ": ERROR: " + ccc::io::Reset + "Fail to link");
        for (size_t i = 0; i < this->status.size(); i++) {
            ccc::io::print(ccc::io::Red + "[" + std::to_string(i) +
                           "]: " + ccc::io::Reset);
            ccc::io::println(this->status[i]);
        }

        // Remove the output file.
        fs::remove(this->output_path / this->name);
        return;
    }
}

void ccc::build_target::compile(const ccc::config& project_cfg,
                                std::vector<std::string>& path) {

    // Set the toolchain.
    this->init(project_cfg);

    if (!this->source_files.empty() && this->obj_path.empty()) {
        this->status.push_back("Object directory path is empty");
        return;
    }

    // Validate every source before starting any compilation task.
    for (const auto& source_file : this->source_files) {
        if (source_file.is_absolute()) {
            this->status.push_back("Source file path is absolute: " +
                                   source_file.string());
            return;
        }
        if (source_file.empty() || source_file.has_root_path()) {
            this->status.push_back("Invalid source file path: " +
                                   source_file.string());
            return;
        }

        const fs::path relative = source_file.lexically_normal();
        if (relative.empty() || relative == fs::path(".")) {
            this->status.push_back("Invalid source file path: " +
                                   source_file.string());
            return;
        }
        if (*relative.begin() == fs::path("..")) {
            this->status.push_back(
                "Source file is outside project directory: " +
                source_file.string());
            return;
        }
    }

    // Add the compile task to the path.
    path.push_back(this->loc_info);

    // Process the dependencies.
    for (auto& [dep, dep_desc] : dependencies) {
        // If the dependency does not exist and the is_compile is true, process
        // it.
        if (!fs::exists(dep->output_path / dep->name) && dep_desc.is_compile) {
            // Process the dependency.
            dep->process(project_cfg, path);
            if (dep->status.size() != 0) {
                this->status.push_back("Fail to compile dependency: " +
                                       dep->loc_info);
                fs::remove(dep->output_path / dep->name);
            }
            ccc::io::println("");
        }

        // Add header paths.
        for (auto& header_folder_path : dep->config.header_folder_paths) {
            this->config.header_folder_paths.push_back(header_folder_path);
        }
    }

    // Print the path.
    std::string msg;
    for (size_t i = 0; i < path.size(); i++) {
        if (i == 0) {
            msg += path[i];
        } else {
            msg += " => " + path[i];
        }
    }
    ccc::io::println(msg);

    // Remove the compile task from the path.
    path.pop_back();

    // Create a thread pool.
    unsigned int core_num = std::thread::hardware_concurrency();
    size_t thread_num =
        this->config.thread_num != 0
            ? this->config
                  .thread_num // Use the this.config thread num if it is not 0.

        : project_cfg.thread_num != 0
            ? project_cfg
                  .thread_num // Use the project.config thread num if it
                              // is not 0 and the this.config thread num is 0.

        : core_num != 0 ? core_num // If all the above variables are set, use
                                   // the kernel number as the parameter.

                        : 1; // If the number of cores cannot be obtained,
                             // set the parameter to 1.

    // Merge the project and task configurations.
    this->config.compile_flags.insert(this->config.compile_flags.end(),
                                      project_cfg.compile_flags.begin(),
                                      project_cfg.compile_flags.end());
    this->config.header_folder_paths.insert(
        this->config.header_folder_paths.end(),
        project_cfg.header_folder_paths.begin(),
        project_cfg.header_folder_paths.end());
    this->config.macros.insert(this->config.macros.end(),
                               project_cfg.macros.begin(),
                               project_cfg.macros.end());

    // Create a mutex and condition variable for thread synchronization.
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> threads;
    std::atomic<size_t> active_threads(0);

    // Iteratively compile all source files.
    for (const auto& source_file : source_files) {
        // Wait until there is an available thread slot.
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return active_threads < thread_num; });

        // Increment the active thread count.
        active_threads++;

        // Launch a new thread to compile the source file.
        threads.emplace_back([this, &project_cfg, source_file,
                              &active_threads, &cv]() {
            compile_source_file(project_cfg, source_file);

            // Decrement the active thread count and notify the waiting
            // threads.
            active_threads--;
            cv.notify_one();
        });
    }

    // Wait for all threads to finish.
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    for (auto& [dep, dep_desc] : dependencies) {
        // If the is_transmit is true, transmit the dependency.
        if (dep_desc.is_transmit) {
            dep->transmit(*this);
        }
    }
}

static std::mutex compile_mtx;
void ccc::build_target::compile_source_file(const ccc::config& project_cfg,
                                            const fs::path& source_file) {

    // Get the obj file path.
    fs::path obj_file_path = this->obj_path / source_file.lexically_normal();
    obj_file_path.replace_extension(
        this->config.toolchain.target_os == windows_os ? ".obj" : ".o");

    // Add the obj file to the list.
    {
        std::lock_guard<std::mutex> lock(compile_mtx);
        obj_files.push_back(obj_file_path);
    }

    // If the obj file exists and is newer than the source file, skip it.
    if (fs::exists(obj_file_path) &&
        fs::last_write_time(source_file) < fs::last_write_time(obj_file_path)) {
        return;
    }

    // Get the target folder(The storage path of the obj file.)
    fs::path target_folder = obj_file_path.parent_path();
    // If the folder doesn't exist, create it.
    if (!fs::is_directory(target_folder)) {
        fs::create_directories(target_folder);
    }

    // Replace the placeholders in the format string.
    auto replacements =
        std::unordered_map<std::string, std::vector<std::string>>{
            {"COMPILER",
             {this->config.toolchain.compiler.length() != 0
                  ? this->config.toolchain.compiler
              : project_cfg.toolchain.compiler.length() != 0
                  ? project_cfg.toolchain.compiler
                  : "g++"}},
            {"SOURCE_FILE", {source_file.string()}},
            {"OBJECT_FILE", {obj_file_path.string()}},
            {"COMPILE_FLAGS",
             {this->config.compile_flags.begin(),
              this->config.compile_flags.end()}},
            {"HEADER_FOLDERS",
             {this->config.header_folder_paths.begin(),
              this->config.header_folder_paths.end()}},
            {"MACROS",
             {this->config.macros.begin(), this->config.macros.end()}},
        };
    std::string cmd =
        this->config.toolchain.compile_format.replace(replacements);

    // Execute the command.
    if (!ccc::io::exec_command(cmd,
                               project_cfg.is_print && this->config.is_print,
                               project_cfg.is_print && this->config.is_print)) {

        std::lock_guard<std::mutex> lock(compile_mtx);
        // Add the fail message to the status.
        this->status.push_back("Fail to compile file: " + source_file.string());
        // Remove the obj file that failed to compile.
        if (std::filesystem::exists(obj_file_path))
            std::filesystem::remove(obj_file_path);
    }
}

void ccc::build_target::add_source_file(const fs::path& file_path) {
    // Keep the original spelling for the compile command.
    this->source_files.emplace_back(file_path);
    return;
}

void ccc::build_target::add_source_files(
    const std::initializer_list<fs::path>& file_paths) {
    for (const auto& file_path : file_paths) {
        this->source_files.emplace_back(file_path);
    }
    return;
}

void ccc::build_target::add_source_files(
    const std::initializer_list<fs::path>& dir_paths,
    const std::initializer_list<std::string>& suffixs, bool recursive) {
    namespace fs = std::filesystem;

    // Convert suffix initializer_list to vector for easier processing
    std::vector<std::string> suffix_vector(suffixs);

    // Process each directory in the input list
    for (const auto& dir : dir_paths) {
        const fs::path& dir_path = dir;

        // Skip invalid directories
        if (!fs::exists(dir_path))
            continue; // Path doesn't exist
        if (!fs::is_directory(dir_path))
            continue; // Path isn't a directory

        std::vector<fs::directory_entry> entries;

        // Collect directory entries (recursive or non-recursive)
        if (recursive) {
            for (const auto& entry :
                 fs::recursive_directory_iterator(dir_path)) {
                entries.emplace_back(entry);
            }
        } else {
            for (const auto& entry : fs::directory_iterator(dir_path)) {
                entries.emplace_back(entry);
            }
        }

        // Process collected directory entries
        for (const auto& entry : entries) {
            // Skip non-files and symlinks
            if (!entry.is_regular_file() || entry.is_symlink())
                continue;

            // Extract file extension (includes the dot, e.g., ".cpp")
            const std::string ext = entry.path().extension().string();

            // Skip files without extensions
            if (!ext.empty()) {
                // Check if extension matches any of the target suffixes
                for (const auto& suffix : suffix_vector) {
                    if (ext == suffix) {

                        source_files.emplace_back(entry.path());
                        break; // No need to check other suffixes
                    }
                }
            }
        }
    }
}
void ccc::build_target::add_source_files(
    const std::initializer_list<fs::path>& dir_paths,
    auto judge(const fs::path&)->bool, bool recursive) {
    namespace fs = std::filesystem;

    // Iterate through each directory path provided in dir_paths
    for (const auto& dir : dir_paths) {
        const fs::path& dir_path = dir;

        // Skip if the directory does not exist or is not a valid directory
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path))
            continue;

        // If recursive flag is true, use recursive_directory_iterator
        if (recursive) {
            for (fs::recursive_directory_iterator it(dir_path);
                 it != fs::recursive_directory_iterator(); ++it) {
                const auto& entry = *it;

                // Check if the entry is a regular file and not a symbolic link
                if (entry.is_regular_file() && !entry.is_symlink()) {
                    // If the judge function returns true for this file, add it
                    // to source_files
                    if (judge(entry.path()))
                        source_files.emplace_back(entry.path());
                }
            }
        } else { // If recursive flag is false, use directory_iterator
            for (fs::directory_iterator it(dir_path);
                 it != fs::directory_iterator(); ++it) {
                const auto& entry = *it;

                // Check if the entry is a regular file and not a symbolic link
                if (entry.is_regular_file() && !entry.is_symlink()) {
                    // If the judge function returns true for this file, add it
                    // to source_files
                    if (judge(entry.path()))
                        source_files.emplace_back(entry.path());
                }
            }
        }
    }
}
void ccc::build_target::remove_source_file(const fs::path& file_path) {
    const fs::path identity = file_path.lexically_normal();
    size_t index = 0;
    for (size_t i = 0; i < source_files.size(); ++i) {
        if (source_files[i].lexically_normal() != identity) {
            // Move non-matching elements to front
            if (index != i) {
                source_files[index] = std::move(source_files[i]);
            }
            ++index;
        }
    }
    // Truncate vector to new size
    source_files.resize(index);
}

void ccc::build_target::remove_source_files(
    const std::initializer_list<fs::path>& file_paths) {
    // Iterate through paths to remove
    for (const auto& path : file_paths) {
        const fs::path identity = path.lexically_normal();
        // Reimplement single removal logic for each path
        size_t index = 0;
        for (size_t i = 0; i < source_files.size(); ++i) {
            if (source_files[i].lexically_normal() != identity) {
                // Compact array by moving kept elements
                if (index != i) {
                    source_files[index] = std::move(source_files[i]);
                }
                ++index;
            }
        }
        source_files.resize(index);
    }
}

int ccc::build_target::remove_source_files(bool (*judge)(const fs::path&)) {
    int removed_count = 0;
    size_t index = 0;

    // Process all elements
    for (size_t i = 0; i < source_files.size(); ++i) {
        if (!judge(source_files[i])) { // Keep non-matching elements
            // Shift elements to maintain order
            if (index != i) {
                source_files[index] = std::move(source_files[i]);
            }
            ++index;
        } else { // Count removed elements
            ++removed_count;
        }
    }
    // Resize container after removal
    source_files.resize(index);
    return removed_count;
}

bool ccc::build_target::find_source_file(const fs::path& file_path) {
    const fs::path identity = file_path.lexically_normal();
    // Implement linear search manually
    for (const auto& path : source_files) {
        if (path.lexically_normal() == identity) {
            return true; // Early return when found
        }
    }
    return false; // Return false if not found
}
