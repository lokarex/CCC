#ifndef __CCC_CMD_NEW_HPP__
#define __CCC_CMD_NEW_HPP__

#include "util/io.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ccc::new_command {
namespace fs = std::filesystem;

struct options {
    fs::path path;
    std::string name;
    bool is_library = false;
};

inline bool valid_name(const std::string& name) {
    if (name.empty() || !((name[0] >= 'a' && name[0] <= 'z') ||
                          (name[0] >= 'A' && name[0] <= 'Z')))
        return false;

    for (char c : name) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-'))
            return false;
    }

    if (name.back() == '-' || name.back() == '_')
        return false;

    std::string upper = name;
    for (char& c : upper) {
        if (c >= 'a' && c <= 'z')
            c = static_cast<char>(c - 'a' + 'A');
    }
    if (upper == "CON" || upper == "PRN" || upper == "AUX" || upper == "NUL")
        return false;
    if (upper.size() == 4 &&
        (upper.substr(0, 3) == "COM" || upper.substr(0, 3) == "LPT") &&
        upper[3] >= '1' && upper[3] <= '9')
        return false;
    return true;
}

inline bool parse_arguments(const std::vector<std::string>& args, options& out,
                            std::string& error) {
    out = options{};
    error.clear();
    bool has_path = false;
    bool has_type = false;
    for (const auto& arg : args) {
        if (arg == "--bin" || arg == "--lib") {
            if (has_type) {
                error = "Specify --bin or --lib only once.";
                return false;
            }
            has_type = true;
            out.is_library = arg == "--lib";
        } else if (!arg.empty() && arg[0] == '-') {
            error = "Unknown option: " + arg;
            return false;
        } else if (has_path) {
            error = "Specify exactly one project path.";
            return false;
        } else if (arg.empty()) {
            error = "Project path cannot be empty.";
            return false;
        } else {
            out.path = fs::path(arg).lexically_normal();
            has_path = true;
        }
    }

    if (!has_path) {
        error = "Project path is required.";
        return false;
    }

    while (out.path.filename().empty() && out.path.has_parent_path() &&
           out.path != out.path.root_path())
        out.path = out.path.parent_path();
    out.name = out.path.filename().string();
    if (!valid_name(out.name)) {
        error = "Invalid project name: " + out.name;
        return false;
    }
    return true;
}

inline std::string cpp_namespace(const std::string& name) {
    std::string result = "ccc_";
    for (char c : name) {
        if (c == '-' || c == '_') {
            if (result.back() != '_')
                result += '_';
        } else {
            result += c;
        }
    }
    return result;
}

inline void write_file(const fs::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot create " + path.string());
    file << content;
    file.close();
    if (!file)
        throw std::runtime_error("Cannot write " + path.string());
}

inline std::string project_source(const options& opts) {
    const std::string source_dir = "./packages/" + opts.name + "/src";
    std::string content = "#include \"ccc/project.h\"\n"
                          "#include <string>\n"
                          "#include <vector>\n\n"
                          "using namespace ccc;\n\n"
                          "void init_project(project* self, std::string, "
                          "std::vector<std::string>) {\n";

    if (opts.is_library) {
        content += "    static library target(\"" + opts.name +
                   "\", static_library, \"" + opts.name +
                   " library\");\n"
                   "    target.add_source_files({\"" +
                   source_dir +
                   "\"}, {\".cpp\"});\n"
                   "    target.add_header_folder_paths({\"./packages/" +
                   opts.name + "/include\"});\n";
    } else {
        content += "    static execution target(\"" + opts.name + "\", \"" +
                   opts.name +
                   " executable\");\n"
                   "    target.add_source_files({\"" +
                   source_dir + "\"}, {\".cpp\"});\n";
    }

    content += "    self->add_task(&target);\n"
               "}\n\n"
               "void exit_project(project*, std::string, "
               "std::vector<std::string>) {}\n\n"
               "project generated_project(\"" +
               opts.name + "\", init_project, exit_project, \"" + opts.name +
               " project\");\n";
    return content;
}

inline void create_project(const options& opts) {
    if (fs::exists(opts.path) || fs::is_symlink(opts.path))
        throw std::runtime_error("Target already exists: " +
                                 opts.path.string());

    bool created = false;
    try {
        created = fs::create_directories(opts.path);
        if (!created)
            throw std::runtime_error("Target already exists: " +
                                     opts.path.string());

        const fs::path package = opts.path / "packages" / opts.name;
        fs::create_directories(opts.path / "tests");
        fs::create_directories(package / "tests");
        fs::create_directories(package / "src");
        if (opts.is_library)
            fs::create_directories(package / "include" / opts.name);

        write_file(opts.path / "project.cpp", project_source(opts));
        write_file(opts.path / ".gitignore", "build/\nproject\nproject.exe\n");
        if (opts.is_library) {
            const std::string ns = cpp_namespace(opts.name);
            write_file(package / "include" / opts.name / "lib.hpp",
                       "#pragma once\n\nnamespace " + ns +
                           " {\nint add(int lhs, int rhs);\n}\n");
            write_file(
                package / "src" / "lib.cpp",
                "#include \"" + opts.name + "/lib.hpp\"\n\nnamespace " + ns +
                    " {\nint add(int lhs, int rhs) { return lhs + rhs; }\n}\n");
        } else {
            write_file(package / "src" / "main.cpp",
                       "#include <iostream>\n\n"
                       "int main() {\n"
                       "    std::cout << \"Hello, " +
                           opts.name +
                           "!\\n\";\n"
                           "    return 0;\n"
                           "}\n");
        }
    } catch (...) {
        if (created) {
            std::error_code cleanup_error;
            fs::remove_all(opts.path, cleanup_error);
        }
        throw;
    }
}
} // namespace ccc::new_command

inline auto new_func = [](std::vector<std::string> args) {
    ccc::new_command::options opts;
    std::string error;
    if (!ccc::new_command::parse_arguments(args, opts, error)) {
        ccc::io::error(error + " Usage: ccc new <path> [--bin|--lib]");
        std::exit(1);
    }
    try {
        ccc::new_command::create_project(opts);
    } catch (const std::exception& e) {
        ccc::io::error(e.what());
        std::exit(1);
    }
    ccc::io::println("Created " +
                     std::string(opts.is_library ? "library" : "executable") +
                     " project at " + opts.path.string());
};

#endif
