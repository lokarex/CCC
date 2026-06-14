#ifndef __CCC_SYSTEM_EXAMPLE_HARNESS_HPP__
#define __CCC_SYSTEM_EXAMPLE_HARNESS_HPP__

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace ccc_system_test {
namespace fs = std::filesystem;

class Example {
  public:
    explicit Example(std::string name)
        : name_(std::move(name)), project_dir_(prepare_example(name_)),
          ccc_home_(work_root() / "ccc_home") {}

    bool build() const {
        if (!prepare_ccc_home())
            return false;

        return std::system(build_command().c_str()) == 0;
    }

    bool run() const { return run(name_); }

    bool run(const std::string& executable_name) const {
        return std::system(executable_path(executable_name).string().c_str()) ==
               0;
    }

    bool has_executable(const std::string& executable_name) const {
        return fs::exists(executable_path(executable_name));
    }

    bool has_static_library(const std::string& library_name) const {
        return fs::exists(project_dir_ / "build" / "lib" /
                          static_library_name(library_name));
    }

  private:
    static fs::path repo_root() { return fs::current_path(); }

    static fs::path work_root() {
        return repo_root() / "build" / "tests" / "systemtest" / "work";
    }

    static fs::path prepare_example(const std::string& name) {
        fs::path source = repo_root() / "examples" / name;
        fs::path dest = work_root() / "examples" / name;

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

    bool prepare_ccc_home() const {
        fs::path home_build = ccc_home_ / "build";
        fs::path source_ccc = repo_ccc_executable();
        if (!fs::exists(source_ccc))
            return false;

        fs::remove_all(ccc_home_);
        fs::create_directories(home_build / "bin");
        std::ofstream(ccc_home_ / "project.cpp").close();

        fs::copy(
            repo_root() / "packages" / "cccsdk" / "include", home_build / "inc",
            fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        fs::copy(repo_root() / "build" / "lib", home_build / "lib",
                 fs::copy_options::recursive |
                     fs::copy_options::overwrite_existing);
        fs::copy_file(source_ccc, ccc_executable(),
                      fs::copy_options::overwrite_existing);

        return true;
    }

    std::string build_command() const {
        fs::path bin_dir = ccc_home_ / "build" / "bin";
        fs::path lib_dir = ccc_home_ / "build" / "lib";

#ifdef _WIN32
        return "cd /d " + project_dir_.string() +
               " && set \"CCC_HOME=" + ccc_home_.string() +
               "\" && set \"PATH=" + bin_dir.string() + ";" + lib_dir.string() +
               ";%PATH%\" && " + ccc_executable().string() + " project && " +
               project_runtime_name() + " build --noprint";
#else
        std::string env = "CCC_HOME=" + ccc_home_.string() +
                          " PATH=" + bin_dir.string() + ":$PATH" +
                          " LD_LIBRARY_PATH=" + lib_dir.string() +
                          ":$LD_LIBRARY_PATH";
        return "cd " + project_dir_.string() + " && " + env + " " +
               ccc_executable().string() + " project && " + env + " " +
               project_runtime_name() + " build --noprint";
#endif
    }

    fs::path ccc_executable() const {
        return ccc_home_ / "build" / "bin" / executable_name("ccc");
    }

    static fs::path repo_ccc_executable() {
        return repo_root() / "build" / "bin" / executable_name("ccc");
    }

    fs::path executable_path(const std::string& executable_name_base) const {
        return project_dir_ / "build" / "bin" /
               executable_name(executable_name_base);
    }

    static std::string executable_name(const std::string& base) {
#ifdef _WIN32
        return base + ".exe";
#else
        return base;
#endif
    }

    static std::string static_library_name(const std::string& base) {
#ifdef _WIN32
        return "lib" + base + ".lib";
#else
        return "lib" + base + ".a";
#endif
    }

    static std::string project_runtime_name() {
#ifdef _WIN32
        return "project.exe";
#else
        return "./project";
#endif
    }

    std::string name_;
    fs::path project_dir_;
    fs::path ccc_home_;
};

} // namespace ccc_system_test

#endif
