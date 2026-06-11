#include "ccc/project.h"
#include "ccc/command.h"
#include "ccc/execution.h"
#include "ccc/info.hpp"
#include "ccc/library.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace ccc;
namespace fs = filesystem;

static project ccc_project(
    "CCC",
    [](project* self, string, vector<string> args) {
        // Set compiler flags
        unordered_set<string> ccc_args(args.begin(), args.end());
        if (ccc_args.find("release") != ccc_args.end()) {
            self->add_compile_flags(
                {"-O2", "-std=c++20", "-W", "-Wall", "-Wextra"});
        } else {
            self->add_compile_flags(
                {"-g", "-std=c++20", "-W", "-Wall", "-Wextra"});
        }

        if (ccc_args.count("--noprint")) {
            self->config.is_print = false;
        }

        /* Describe the executable program ccc. */
        static execution ccc(
            "ccc",
            "The ccc does not rely on cccruntime and cccsdk, but its working "
            "principle is to link project.cpp, cccruntime, and cccsdk in a "
            "directory with project.cpp to generate and call project, and call "
            "defautl_project in a directory without project.cpp.");
        ccc.add_source_files({"./packages/cccboot/src/main.cpp"});
        ccc.add_header_folder_paths(
            {"./packages/cccsdk/include", "./packages/cccboot/include"});
        self->add_task(&ccc);

        /* Describe the library file cccsdk. */
        static library cccsdk(
            "cccsdk", shared_library,
            "The cccsdk is the core framework of the project, "
            "defining how to describe the project when using ccc.");
        cccsdk.add_source_files({"./packages/cccsdk/src/"}, {".cpp"});
        cccsdk.add_header_folder_paths({"./packages/cccsdk/include"});
        self->add_task(&cccsdk);

        /* Describe the library file cccruntime. */
        static library cccruntime(
            "cccruntime", static_library,
            "The cccruntime relies on cccsdk and provides the "
            "runtime entry point for compiled projects.");
        cccruntime.add_source_files(
            {"./packages/cccruntime/src/cccruntime.cpp"});
        cccruntime.add_dependency(&cccsdk, false);
        self->add_task(&cccruntime);
    },
    [](project*, string cmd, vector<string>) {
        // Copy the cccunit/inc directory to the build/inc directory when
        // running the build command.
        if (cmd == "build" && fs::exists("./packages/cccsdk/include") &&
            fs::is_directory("./packages/cccsdk/include")) {

            function<void(const fs::path& source, const fs::path& destination)>
                copy_directory = [&](const fs::path& source,
                                     const fs::path& destination) -> void {
                try {
                    // Check if the source path exists and is a directory
                    if (!fs::exists(source) || !fs::is_directory(source)) {
                        cerr << "Source directory does not exist or is not a "
                                "directory."
                             << endl;
                        return;
                    }

                    // Create target directory
                    if (!fs::exists(destination)) {
                        fs::create_directories(destination);
                    }

                    // Traverse all entries in the source directory
                    for (const auto& entry : fs::directory_iterator(source)) {
                        const auto& path = entry.path();
                        const auto& filename = path.filename();
                        const auto dest_path = destination / filename;

                        if (fs::is_directory(path)) {
                            // If it is a directory, recursively copy
                            copy_directory(path, dest_path);
                        } else if (fs::is_regular_file(path)) {
                            // If it is a file, remove the existing file and
                            // copy the file
                            if (fs::exists(dest_path)) {
                                fs::remove(dest_path);
                            }
                            fs::copy_file(path, dest_path,
                                          fs::copy_options::overwrite_existing);
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    cerr << "Filesystem error: " << e.what() << endl;
                } catch (const exception& e) {
                    cerr << "General error: " << e.what() << endl;
                }
            };
            copy_directory("./packages/cccsdk/include", "./build/inc");
        }

        // Clean the build/inc directory when running the clean command.
        if (cmd == "clean") {
            fs::remove_all("./build/inc");
        }
    },
    "\n" + info::help_msg +
        "Note: For the built-in commands provided above, you can "
        "use them directly in any project.\n" +

        "\nExtended commands:\n"
        "    debug                Compile the ccc in debug mode.\n"
        "    release              Compile the ccc in release mode.\n"
        "    help                 Print the help message about this "
        "project(ccc).\n"
        "    line                 Print the number of lines of code for the "
        "ccc project.\n"
        "    unittest [pkg]       Build and run unit tests for CCC "
        "components.\n"
        "Extended arguments:\n"
        "    --noprint            Don't generate any output when compile the "
        "ccc.\n"
        "Note: For the extended commands and arguments, you can use them in "
        "the this project. But if you want to use them in other projects, you "
        "need to achieve them by yourself.\n");

command debug_cmd(
    "debug",
    [](vector<string> args) {
        cout << "Compile the ccc in debug mode." << endl;
        std::string cmd = "ccc build debug";
        for (size_t i = 0; i < args.size(); i++) {
            cmd += " " + args[i];
        }
#ifdef _WIN32
        if (system(cmd.c_str()) != 0) {
            exit(-1);
        }
#endif
#ifdef __linux__
        if (system(("bash -c '" + cmd + "'").c_str()) != 0) {
            exit(-1);
        }
#endif
    },
    "Compile the ccc in debug mode.");

command release_cmd(
    "release",
    [](vector<string> args) {
        cout << "Compile the ccc in release mode." << endl;
        std::string cmd = "ccc build release";
        for (size_t i = 0; i < args.size(); i++) {
            cmd += " " + args[i];
        }
#ifdef _WIN32
        if (system(cmd.c_str()) != 0) {
            exit(-1);
        }
#endif
#ifdef __linux__
        if (system(("bash -c '" + cmd + "'").c_str()) != 0) {
            exit(-1);
        }
#endif
    },
    "Compile the ccc in release mode.");

command help_cmd(
    "help",
    [](vector<string> args) {
        std::string cmd = "ccc desc CCC";
        for (size_t i = 0; i < args.size(); i++) {
            cmd += " " + args[i];
        }
#ifdef _WIN32
        system(cmd.c_str());
#endif
#ifdef __linux__
        system(("bash -c '" + cmd + "'").c_str());
#endif
    },
    "Print the help message about this project(ccc).");

command line_cmd(
    "line",
    [](vector<string>) {
        int total_lines = 0;
        vector<string> directories = {
            "./packages/cccboot", "./packages/cccruntime", "./packages/cccsdk"};
        unordered_set<string> valid_ext = {".cpp", ".h", ".hpp", ".c"};

        auto count_lines = [&](const fs::path& path) {
            ifstream file(path);
            if (!file.is_open())
                return 0;

            int count = 0;
            string line;
            bool in_block_comment = false;
            while (getline(file, line)) {
                // Remove whitespace
                line.erase(remove_if(line.begin(), line.end(), ::isspace),
                           line.end());

                // Single-Line annotation
                if (line.length() >= 2 && line[0] == '/' && line[1] == '/')
                    continue;
                // Single-line block annotation
                if (line.length() >= 4 && line[0] == '/' && line[1] == '*' &&
                    line[line.length() - 2] == '*' &&
                    line[line.length() - 1] == '/')
                    continue;

                // Processing multi line block annotations
                if (line.length() >= 2 && line[0] == '/' && line[1] == '*')
                    in_block_comment = true;
                if (in_block_comment) {
                    if (line.length() >= 2 && line[line.length() - 2] == '*' &&
                        line[line.length() - 1] == '/')
                        in_block_comment = false;
                    continue;
                }

                if (!line.empty())
                    count++;
            }
            return count;
        };

        for (const auto& dir : directories) {
            int dir_lines = 0;
            cout << "Counting lines in: " << dir << endl;

            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file() &&
                    valid_ext.count(entry.path().extension().string())) {
                    int lines = count_lines(entry.path());
                    dir_lines += lines;
                    cout << "  " << entry.path().filename() << ": " << lines
                         << endl;
                }
            }

            cout << "Total for " << fs::path(dir).filename() << ": "
                 << dir_lines << "\n\n";
            total_lines += dir_lines;
        }

        cout << "================================\n";
        cout << "Grand total lines of code: " << total_lines << endl;
    },
    "Print the number of lines of code for the ccc project.");

command unittest_cmd(
    "unittest",
    [](vector<string> args) {
        string component = args.empty() ? "" : args[0];

        struct test_def {
            string pkg;
            string mod;
            vector<string> extra_sources;
        };
        vector<test_def> all_tests = {
            {"cccsdk", "config", {"packages/cccsdk/src/ccc/config.cpp"}},
            {"cccsdk", "toolchain", {"packages/cccsdk/src/ccc/toolchain.cpp"}},
            {"cccsdk",
             "command",
             {"packages/cccsdk/src/ccc/command.cpp",
              "packages/cccsdk/src/ccc/global.cpp",
              "packages/cccsdk/src/util/io.cpp"}},
            {"cccsdk",
             "global",
             {"packages/cccsdk/src/ccc/global.cpp",
              "packages/cccsdk/src/ccc/command.cpp",
              "packages/cccsdk/src/util/io.cpp"}},
        };

        ccc::config project_cfg;
        project_cfg.compile_flags = {"-g", "-std=c++20", "-W", "-Wall",
                                     "-Wextra"};
        project_cfg.header_folder_paths = {
            "./packages/cccsdk/include",
            "./vendor/doctest",
        };
        project_cfg.is_print = false;

        int passed = 0;
        int failed = 0;

        for (auto& td : all_tests) {
            if (!component.empty() && td.pkg != component)
                continue;

            string task_name = "unittest_" + td.pkg + "_" + td.mod;

            cout << "[unittest] " << td.pkg << "/" << td.mod << " ... building"
                 << endl;

            ccc::execution t(task_name, "unit test: " + td.pkg + "/" + td.mod);
            t.add_source_file("packages/" + td.pkg + "/tests/test_" + td.mod +
                              ".cpp");
            for (auto& src : td.extra_sources)
                t.add_source_file(src);

            vector<string> path;
            try {
                t.process(project_cfg, path);
            } catch (...) {
                failed++;
                continue;
            }

            if (!t.status.empty()) {
                cout << "[unittest] " << td.pkg << "/" << td.mod
                     << " ... build FAILED" << endl
                     << endl;
                failed++;
                continue;
            }

            cout << "[unittest] " << td.pkg << "/" << td.mod << " ... running"
                 << endl;

            fs::path exe_path = fs::path("build") / "bin" / t.name;
            int exit_code = -1;
#ifdef _WIN32
            exit_code = system(exe_path.string().c_str());
#endif
#ifdef __linux__
            exit_code = system(("bash -c '" + exe_path.string() + "'").c_str());
#endif

            if (exit_code == 0) {
                cout << "[unittest] " << td.pkg << "/" << td.mod
                     << " ... PASSED" << endl
                     << endl;
                passed++;
            } else {
                cout << "[unittest] " << td.pkg << "/" << td.mod
                     << " ... FAILED (exit " << exit_code << ")" << endl
                     << endl;
                failed++;
            }
        }

        cout << "================================\n";
        cout << "Total: " << (passed + failed) << ", Passed: " << passed
             << ", Failed: " << failed << endl;
        if (failed > 0)
            exit(-1);
    },
    "Build and run unit tests for CCC components.");