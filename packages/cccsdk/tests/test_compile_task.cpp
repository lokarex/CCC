#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/execution.h"
#include "ccc/library.h"
#include "ccc/toolchain.h"
#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace ccc;
namespace fs = std::filesystem;

namespace {
bool contains(const std::vector<std::string>& values,
              const std::string& expected) {
    for (const auto& value : values) {
        if (value == expected)
            return true;
    }
    return false;
}
} // namespace

TEST_CASE("compile_task source file helpers add find and remove files") {
    execution task("__test_sources", "source helper test");

    task.add_source_file("a.cpp");
    task.add_source_files({"b.cpp", "a.cpp"});

    CHECK(task.find_source_file("a.cpp"));
    CHECK(task.find_source_file("b.cpp"));

    task.remove_source_file("a.cpp");
    CHECK_FALSE(task.find_source_file("a.cpp"));
    CHECK(task.find_source_file("b.cpp"));

    task.remove_source_files({"b.cpp", "missing.cpp"});
    CHECK(task.source_files.empty());
}

TEST_CASE("compile_task collects source files recursively by suffix") {
    fs::path root = fs::path("build") / "tests" / "unittest" / "work" /
                    "compile_task" / "sources";
    fs::remove_all(root);
    fs::create_directories(root / "nested");

    std::ofstream(root / "main.cpp") << "int main() { return 0; }\n";
    std::ofstream(root / "nested" / "helper.cpp")
        << "int helper() { return 1; }\n";
    std::ofstream(root / "nested" / "helper.h") << "#pragma once\n";

    execution recursive("__test_recursive_sources", "recursive source test");
    recursive.add_source_files({root.string()}, {".cpp"}, true);

    CHECK(contains(recursive.source_files,
                   (root / "main.cpp").lexically_normal().string()));
    CHECK(
        contains(recursive.source_files,
                 (root / "nested" / "helper.cpp").lexically_normal().string()));
    CHECK_FALSE(
        contains(recursive.source_files,
                 (root / "nested" / "helper.h").lexically_normal().string()));

    execution flat("__test_flat_sources", "flat source test");
    flat.add_source_files({root.string()}, {".cpp"}, false);

    CHECK(contains(flat.source_files,
                   (root / "main.cpp").lexically_normal().string()));
    CHECK_FALSE(
        contains(flat.source_files,
                 (root / "nested" / "helper.cpp").lexically_normal().string()));
}

TEST_CASE("compile_task propagates dependency header paths before compiling") {
    config cfg;
    cfg.is_print = false;
    cfg.toolchain = built_in_toolchain::gnu_toolchain();

    library dep("__test_header_dep", static_library, "dependency");
    dep.add_header_folder_paths({"./dep/include"});

    execution consumer("__test_header_consumer", "consumer");
    consumer.add_dependency(&dep, false, false);

    std::vector<std::string> path;
    consumer.compile(cfg, path);

    CHECK(contains(consumer.config.header_folder_paths, "./dep/include"));
    CHECK(consumer.status.empty());
}
