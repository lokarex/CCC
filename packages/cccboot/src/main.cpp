#include "cccboot/util/file.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/* The name of the cccruntime library. */
std::string ccruntime_name =
#ifdef _WIN32
    "libcccruntime.lib";
#endif
#ifdef __linux__
"libcccruntime.a";
#endif

/* The name of the cccsdk library. */
std::string cccsdk_name =
#ifdef _WIN32
    "cccsdk.dll";
#endif
#ifdef __linux__
"libcccsdk.so";
#endif

/* The home directory of the ccc. */
std::string ccc_home;

void process_project(std::vector<std::string> source_list, std::string target,
                     std::string compile_flags, std::string run_args) {
    // Obtain the source files.
    std::string source;
    for (auto& source_file : source_list) {
        source += source_file + " ";
    }

    // Compile the project.cpp
    std::string compiler = "g++";
    std::string compile_cmd = compiler + " -o " +
                              // Target
                              target + " " +
                              // cccruntime library
                              "-lcccruntime " +
                              // Source
                              source + " " +
                              // cccsdk library
                              "-lcccsdk " +
                              // Include path
                              "-I" + ccc_home + "/build/inc " +
                              // Library path
                              "-L " + ccc_home + "/build/lib " +
                              // C++20
                              "-std=c++20 " +
                              // Compile flags
                              compile_flags;
    for (auto& source_file : source_list) {
        if (!fs::exists(target) ||
            !compareFileModificationTime(source_file, target) ||
            !compareFileModificationTime(
                ccc_home + "/build/lib/" + ccruntime_name, target) ||
            !compareFileModificationTime(ccc_home + "/build/lib/" + cccsdk_name,
                                         target)) {
            if (std::system(compile_cmd.c_str()) != 0) {
                std::exit(1);
            }
            break;
        }
    }
    // Run the project.exe
#ifdef __linux__
    std::string run_cmd = "bash -c '" + target + " " + run_args + "'";
#elif _WIN32
    std::string run_cmd = target + " " + run_args;
#endif
    if (std::system(run_cmd.c_str()) != 0) {
        std::exit(1);
    }
}

int main(int argc, char** argv) {
    // Get the ccc home directory.
    try {
        ccc_home = std::getenv("CCC_HOME");
    } catch (...) {
        std::cout << "ccc: error: CCC_HOME environment variable is not set."
                  << std::endl;
        std::exit(1);
    }
    // If there is not a project.cpp file in the ~/.ccc directory, create it.
    if (!fs::exists(ccc_home + "/project.cpp"))
        std::ofstream(ccc_home + "/project.cpp").close();

    std::vector<std::string> source_list{ccc_home + "/project.cpp"};

    // Parse the command line arguments.
    std::string compile_flags;
    std::string run_args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        // Obtain the compile flags(Used to compile the project.cpp file).
        if (arg.find("flags=") == 0) {
            size_t eq_pos = arg.find('=');
            std::string flags_value = arg.substr(eq_pos + 1);
            compile_flags += " " + flags_value;
        }
        // Obtain the run arguments(Used to run the project.exe).
        else {
            // Special case for 'project' command
            if (run_args.size() == 0 && arg == "project") {
#ifdef __linux__
                std::string bin = "project";
#elif _WIN32
                std::string bin = "project.exe";
#endif
                if (fs::exists(bin))
                    fs::remove(bin);
                else if (fs::exists(ccc_home + "/" + bin))
                    fs::remove(ccc_home + "/" + bin);
            }

            run_args += " " + arg;
        }
    }

    // If there is a project.cpp file, compile and run it.
    if (fs::exists("./project.cpp")) {
        source_list.push_back("./project.cpp");
#ifdef __linux__
        process_project(source_list, "./project", compile_flags, run_args);
#elif _WIN32
        process_project(source_list, "project.exe", compile_flags, run_args);
#endif
    }

    // If there is no project.cpp file, use the default project.cpp file(Located
    // in the ~/.ccc directory).
    else {
#ifdef __linux__
        process_project(source_list, ccc_home + "/project", compile_flags,
                        run_args);
#elif _WIN32
        process_project(source_list, ccc_home + "/project.exe", compile_flags,
                        run_args);
#endif
    }

    return 0;
}