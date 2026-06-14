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
            fs::remove_all("./build/tests");
            fs::remove_all("./build/obj/unittest");
            fs::remove_all("./build/obj/systemtest");

            auto remove_legacy_test_artifacts = [](const fs::path& root) {
                if (!fs::exists(root) || !fs::is_directory(root))
                    return;

                for (const auto& entry : fs::directory_iterator(root)) {
                    string name = entry.path().filename().string();
                    if (name.starts_with("unittest_") ||
                        name.starts_with("systemtest_")) {
                        fs::remove_all(entry.path());
                    }
                }
            };

            remove_legacy_test_artifacts("./build");
            remove_legacy_test_artifacts("./build/bin");
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
        "    unittest [target]    Build and run unit tests for CCC "
        "components.\n"
        "    systemtest|systest [suite]\n"
        "                         Build and run system smoke tests for CCC "
        "workflows.\n"
        "    test [kind] [target]\n"
        "                         Build and run CCC tests.\n"
        "Extended arguments:\n"
        "    --noprint            Don't generate any output when compile the "
        "ccc.\n"
        "Note: For the extended commands and arguments, you can use them in "
        "the this project. But if you want to use them in other projects, you "
        "need to achieve them by yourself.\n");

void run_debug_build(const vector<string>& args) {
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
}

command debug_cmd(
    "debug",
    [](vector<string> args) { run_debug_build(args); },
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

namespace {
struct ccc_test_def {
    string group;
    string name;
    string file;
    vector<string> extra_sources;
};

struct ccc_test_result {
    int passed = 0;
    int failed = 0;

    int total() const { return passed + failed; }

    void merge(const ccc_test_result& other) {
        passed += other.passed;
        failed += other.failed;
    }
};

vector<ccc_test_def> unit_tests() {
    return {
        {"cccsdk",
         "config",
         "packages/cccsdk/tests/test_config.cpp",
         {"packages/cccsdk/src/ccc/config.cpp"}},
        {"cccsdk",
         "toolchain",
         "packages/cccsdk/tests/test_toolchain.cpp",
         {"packages/cccsdk/src/ccc/toolchain.cpp"}},
        {"cccsdk",
         "command",
         "packages/cccsdk/tests/test_command.cpp",
         {"packages/cccsdk/src/ccc/command.cpp",
          "packages/cccsdk/src/ccc/global.cpp",
          "packages/cccsdk/src/util/io.cpp"}},
        {"cccsdk",
         "global",
         "packages/cccsdk/tests/test_global.cpp",
         {"packages/cccsdk/src/ccc/global.cpp",
          "packages/cccsdk/src/ccc/command.cpp",
          "packages/cccsdk/src/util/io.cpp"}},
        {"cccsdk",
         "compile_task",
         "packages/cccsdk/tests/test_compile_task.cpp",
         {"packages/cccsdk/src/ccc/compile_task.cpp",
          "packages/cccsdk/src/ccc/config.cpp",
          "packages/cccsdk/src/ccc/command.cpp",
          "packages/cccsdk/src/ccc/global.cpp",
          "packages/cccsdk/src/ccc/library.cpp",
          "packages/cccsdk/src/ccc/execution.cpp",
          "packages/cccsdk/src/ccc/toolchain.cpp",
          "packages/cccsdk/src/util/io.cpp"}},
        {"cccsdk",
         "execution",
         "packages/cccsdk/tests/test_execution.cpp",
         {"packages/cccsdk/src/ccc/compile_task.cpp",
          "packages/cccsdk/src/ccc/config.cpp",
          "packages/cccsdk/src/ccc/command.cpp",
          "packages/cccsdk/src/ccc/global.cpp",
          "packages/cccsdk/src/ccc/execution.cpp",
          "packages/cccsdk/src/ccc/toolchain.cpp",
          "packages/cccsdk/src/util/io.cpp"}},
        {"cccsdk",
         "library",
         "packages/cccsdk/tests/test_library.cpp",
         {"packages/cccsdk/src/ccc/compile_task.cpp",
          "packages/cccsdk/src/ccc/config.cpp",
          "packages/cccsdk/src/ccc/command.cpp",
          "packages/cccsdk/src/ccc/global.cpp",
          "packages/cccsdk/src/ccc/library.cpp",
          "packages/cccsdk/src/ccc/execution.cpp",
          "packages/cccsdk/src/ccc/toolchain.cpp",
          "packages/cccsdk/src/util/io.cpp"}},
        {"cccsdk",
         "project",
         "packages/cccsdk/tests/test_project.cpp",
         {"packages/cccsdk/src/ccc/compile_task.cpp",
          "packages/cccsdk/src/ccc/config.cpp",
          "packages/cccsdk/src/ccc/command.cpp",
          "packages/cccsdk/src/ccc/global.cpp",
          "packages/cccsdk/src/ccc/project.cpp",
          "packages/cccsdk/src/ccc/execution.cpp",
          "packages/cccsdk/src/ccc/toolchain.cpp",
          "packages/cccsdk/src/util/io.cpp"}},
        {"cccsdk",
         "io",
         "packages/cccsdk/tests/test_io.cpp",
         {"packages/cccsdk/src/util/io.cpp"}},
    };
}

vector<ccc_test_def> system_tests() {
    return {
        {"examples", "hello_world", "tests/system/test_hello_world.cpp", {}},
        {"examples", "math_lib", "tests/system/test_math_lib.cpp", {}},
        {"examples", "my_math", "tests/system/test_my_math.cpp", {}},
    };
}

string test_display_name(const ccc_test_def& test) {
    return test.group + "/" + test.name;
}

bool matches_test_filter(const ccc_test_def& test, const string& filter) {
    return filter.empty() || test.group == filter || test.name == filter;
}

ccc_test_result run_registered_tests(const string& command_name,
                                     const string& unknown_target_name,
                                     const string& obj_root,
                                     const vector<ccc_test_def>& all_tests,
                                     const string& filter) {
    ccc::config project_cfg;
    project_cfg.compile_flags = {"-g", "-std=c++20", "-W", "-Wall", "-Wextra"};
    project_cfg.header_folder_paths = {
        "./packages/cccsdk/include",
        "./vendor/doctest",
    };
    project_cfg.is_print = false;

    ccc_test_result result;
    int matched = 0;

    for (const auto& td : all_tests) {
        if (!matches_test_filter(td, filter))
            continue;
        matched++;

        string display_name = test_display_name(td);
        string task_name = command_name + "_" + td.group + "_" + td.name;
        fs::path test_root = fs::path("build") / "tests" / obj_root;
        fs::path test_bin = test_root / "bin";

        cout << "[" << command_name << "] " << display_name << " ... building"
             << endl;

        ccc::execution t(task_name, command_name + ": " + display_name);
        t.output_path = test_bin.string();
        t.obj_path = (test_root / "obj" / td.group / td.name).string();
        fs::remove_all(t.obj_path);
        fs::remove(test_bin / task_name);
        fs::remove(test_bin / (task_name + ".exe"));
        t.add_source_file(td.file);
        for (auto& src : td.extra_sources)
            t.add_source_file(src);

        vector<string> path;
        try {
            t.process(project_cfg, path);
        } catch (...) {
            result.failed++;
            continue;
        }

        if (!t.status.empty()) {
            cout << "[" << command_name << "] " << display_name
                 << " ... build FAILED" << endl
                 << endl;
            result.failed++;
            continue;
        }

        cout << "[" << command_name << "] " << display_name << " ... running"
             << endl;

        fs::path exe_path = fs::path(t.output_path) / t.name;
        int exit_code = -1;
#ifdef _WIN32
        exit_code = system(exe_path.string().c_str());
#endif
#ifdef __linux__
        exit_code = system(("bash -c '" + exe_path.string() + "'").c_str());
#endif

        if (exit_code == 0) {
            cout << "[" << command_name << "] " << display_name << " ... PASSED"
                 << endl
                 << endl;
            result.passed++;
        } else {
            cout << "[" << command_name << "] " << display_name
                 << " ... FAILED (exit " << exit_code << ")" << endl
                 << endl;
            result.failed++;
        }
    }

    if (matched == 0) {
        cerr << "[" << command_name << "] unknown " << unknown_target_name
             << ": " << filter << endl;
        result.failed++;
        return result;
    }

    cout << "================================\n";
    cout << "Total: " << result.total() << ", Passed: " << result.passed
         << ", Failed: " << result.failed << endl;
    return result;
}

void exit_if_tests_failed(const ccc_test_result& result) {
    if (result.failed > 0)
        exit(-1);
}

ccc_test_result run_system_tests(const string& filter) {
    run_debug_build({});
    return run_registered_tests("systemtest", "system test target",
                                "systemtest", system_tests(), filter);
}
} // namespace

command unittest_cmd(
    "unittest",
    [](vector<string> args) {
        string filter = args.empty() ? "" : args[0];
        ccc_test_result result = run_registered_tests(
            "unittest", "unit test target", "unittest", unit_tests(), filter);
        exit_if_tests_failed(result);
    },
    "Build and run unit tests for CCC components.");

command systemtest_cmd(
    {"systemtest", "systest"},
    [](vector<string> args) {
        string filter = args.empty() ? "" : args[0];
        ccc_test_result result = run_system_tests(filter);
        exit_if_tests_failed(result);
    },
    "Build and run system smoke tests for CCC workflows.");

command test_cmd(
    "test",
    [](vector<string> args) {
        ccc_test_result result;

        if (args.empty()) {
            result.merge(run_registered_tests("unittest", "unit test target",
                                              "unittest", unit_tests(), ""));
            result.merge(run_system_tests(""));
            exit_if_tests_failed(result);
            return;
        }

        if (args.size() > 2) {
            cerr << "[test] usage: test [unit|system] [target]" << endl;
            exit(-1);
        }

        string kind = args[0];
        string filter = args.size() == 2 ? args[1] : "";

        if (kind == "unit") {
            result = run_registered_tests("unittest", "unit test target",
                                          "unittest", unit_tests(), filter);
        } else if (kind == "system") {
            result = run_system_tests(filter);
        } else {
            cerr << "[test] unknown test kind: " << kind
                 << ". Known kinds: unit, system" << endl;
            exit(-1);
        }

        exit_if_tests_failed(result);
    },
    "Build and run CCC tests.");
