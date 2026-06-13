#ifndef __CCC_SYSTEM_EXAMPLE_HARNESS_HPP__
#define __CCC_SYSTEM_EXAMPLE_HARNESS_HPP__

#include <filesystem>
#include <fstream>
#include <string>

namespace ccc_system_test {
namespace fs = std::filesystem;

inline std::string quote(const fs::path& path) {
#ifdef _WIN32
    return "\"" + path.string() + "\"";
#else
    std::string raw = path.string();
    std::string escaped = "'";
    for (char ch : raw) {
        if (ch == '\'')
            escaped += "'\\''";
        else
            escaped += ch;
    }
    escaped += "'";
    return escaped;
#endif
}

inline fs::path copy_example(const std::string& name) {
    fs::path repo = fs::current_path();
    fs::path source = repo / "examples" / name;
    fs::path dest =
        repo / "build" / "tests" / "systemtest" / "work" / "examples" / name;

    fs::remove_all(dest);
    fs::create_directories(dest.parent_path());
    fs::copy(source, dest,
             fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);
    fs::remove_all(dest / "build");
    fs::remove(dest / "project");
    fs::remove(dest / "project.exe");
    return dest;
}

inline fs::path prepare_ccc_home() {
    fs::path repo = fs::current_path();
    fs::path home =
        repo / "build" / "tests" / "systemtest" / "work" / "ccc_home";
    fs::path home_build = home / "build";

    fs::remove_all(home);
    fs::create_directories(home_build);
    std::ofstream(home / "project.cpp").close();

    fs::copy(repo / "packages" / "cccsdk" / "include", home_build / "inc",
             fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);
    fs::copy(repo / "build" / "lib", home_build / "lib",
             fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);

    return home;
}

inline fs::path ccc_executable() {
    fs::path exe = fs::current_path() / "build" / "bin" / "ccc";
#ifdef _WIN32
    exe += ".exe";
#endif
    return exe;
}

inline std::string project_runtime_name() {
#ifdef _WIN32
    return "project.exe";
#else
    return "./project";
#endif
}

inline std::string ccc_project_build_command(const fs::path& project_dir) {
    fs::path ccc_home = prepare_ccc_home();
    fs::path ccc = ccc_executable();
    fs::path lib_dir = ccc_home / "build" / "lib";

#ifdef _WIN32
    return "cd /d " + quote(project_dir) +
           " && set \"CCC_HOME=" + ccc_home.string() +
           "\" && set \"PATH=" + lib_dir.string() + ";%PATH%\" && " +
           quote(ccc) + " project && " + project_runtime_name() +
           " build --noprint";
#else
    return "cd " + quote(project_dir) + " && CCC_HOME=" + quote(ccc_home) +
           " LD_LIBRARY_PATH=" + quote(lib_dir) + ":$LD_LIBRARY_PATH " +
           quote(ccc) + " project && CCC_HOME=" + quote(ccc_home) +
           " LD_LIBRARY_PATH=" + quote(lib_dir) + ":$LD_LIBRARY_PATH " +
           project_runtime_name() + " build --noprint";
#endif
}

inline std::string run_command(const fs::path& exe) { return quote(exe); }

inline std::string exe_name(const std::string& base) {
#ifdef _WIN32
    return base + ".exe";
#else
    return base;
#endif
}

inline std::string static_library_name(const std::string& base) {
#ifdef _WIN32
    return "lib" + base + ".lib";
#else
    return "lib" + base + ".a";
#endif
}

} // namespace ccc_system_test

#endif
