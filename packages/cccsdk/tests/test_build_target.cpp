#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"

#include "ccc/build_target.h"
#include "ccc/config.h"
#include "ccc/toolchain.h"

#include <filesystem>
#include <fstream>
#include <source_location>
#include <string>

using namespace ccc;
namespace fs = std::filesystem;

namespace {
class test_target : public build_target {
  public:
    test_target(std::string name, std::string description,
                std::source_location loc = std::source_location::current())
        : build_target(name, description, loc) {}

    void init(const ccc::config&) override {}

    void link(const ccc::config&) override {}

    void transmit(ccc::build_target&) override {}
};

TEST_CASE("build_target adds one source file") {
    test_target target("test_build_target_adds_one_source_file",
                       "add one source file");

    REQUIRE_FALSE(target.find_source_file("main.cpp"));

    target.add_source_file("main.cpp");
    CHECK(target.find_source_file("main.cpp"));
    CHECK_FALSE(target.find_source_file("ohter.cpp"));
}

TEST_CASE("build_target matches equivalent source path spellings") {
    test_target target("test_build_target_normalizes_source_paths",
                       "normalize source paths");
    target.add_source_file("./src/../src/main.cpp");

    CHECK(target.find_source_file("src/main.cpp"));
    target.remove_source_file("src/main.cpp");
    CHECK(target.source_files.empty());

    target.add_source_files({"./src/a.cpp", "src/../src/b.cpp"});
    target.remove_source_files({"src/a.cpp", "src/b.cpp"});
    CHECK(target.source_files.empty());
}

TEST_CASE("build_target copy keeps path values independent") {
    test_target original("test_build_target_copy_paths", "copy paths");
    original.output_path = "build/bin";
    original.obj_path = "build/obj";
    original.add_source_file("./src/main.cpp");
    original.obj_files.emplace_back("build/obj/src/main.o");

    test_target copy(original);
    original.output_path = "elsewhere";
    original.source_files.clear();
    original.obj_files.clear();

    CHECK(copy.output_path == fs::path("build/bin"));
    CHECK(copy.obj_path == fs::path("build/obj"));
    CHECK(copy.source_files == std::vector<fs::path>{"./src/main.cpp"});
    CHECK(copy.obj_files == std::vector<fs::path>{"build/obj/src/main.o"});
}

TEST_CASE("build_target rejects a source path outside the project") {
    test_target target("test_build_target_rejects_parent_source",
                       "reject source outside project");
    target.config.toolchain = built_in_toolchain::gnu_toolchain();
    target.obj_path = "build/tests/unittest/work/build_target/rejected_obj";
    fs::remove_all(target.obj_path);
    target.add_source_file("packages/cccsdk/tests/test_build_target.cpp");
    target.add_source_file("../outside.cpp");

    config project_cfg;
    project_cfg.is_print = false;
    std::vector<std::string> trace;
    target.compile(project_cfg, trace);

    REQUIRE_FALSE(target.status.empty());
    CHECK(target.status[0].find("outside") != std::string::npos);
    CHECK(target.obj_files.empty());
    CHECK_FALSE(fs::exists(target.obj_path));
}

TEST_CASE("build_target permits no-source targets without an object directory") {
    test_target target("test_build_target_no_source_object_path",
                       "no source files");
    target.obj_path.clear();

    config project_cfg;
    project_cfg.is_print = false;
    std::vector<std::string> trace;
    target.compile(project_cfg, trace);

    CHECK(target.status.empty());
}

TEST_CASE("build_target rejects an absolute source path") {
    test_target target("test_build_target_rejects_absolute_source",
                       "reject absolute source path");
    target.config.toolchain = built_in_toolchain::gnu_toolchain();
    target.obj_path = "build/tests/unittest/work/build_target/rejected_obj";
    fs::remove_all(target.obj_path);
    target.add_source_file("packages/cccsdk/tests/test_build_target.cpp");
    target.add_source_file(
        fs::absolute("packages/cccsdk/tests/test_build_target.cpp").string());

    config project_cfg;
    project_cfg.is_print = false;
    std::vector<std::string> trace;
    target.compile(project_cfg, trace);

    REQUIRE_FALSE(target.status.empty());
    CHECK(target.status[0].find("absolute") != std::string::npos);
    CHECK(target.obj_files.empty());
    CHECK_FALSE(fs::exists(target.obj_path));
}

TEST_CASE("build_target places normalized objects below obj_path") {
    const fs::path root = "build/tests/unittest/work/build_target/path_layout";
    fs::remove_all(root);
    const fs::path source = root / "src" / "main.cpp";
    fs::create_directories(source.parent_path());
    std::ofstream(source) << "int answer() { return 42; }\n";

    test_target target("test_build_target_object_layout", "object layout");
    target.config.toolchain = built_in_toolchain::gnu_toolchain();
    target.config.toolchain.compile_format =
        target.config.toolchain.execution_compile_format;
    target.obj_path = root / "obj";
    target.add_source_file((root / "src" / ".." / "src" / "main.cpp").string());

    config project_cfg;
    project_cfg.is_print = false;
    std::vector<std::string> trace;
    target.compile(project_cfg, trace);

    std::string errors;
    for (const auto& message : target.status)
        errors += message + "\n";
    INFO(errors);
    REQUIRE(target.status.empty());
    REQUIRE(target.obj_files.size() == 1);
    fs::path expected = root / "obj" / root / "src" / "main.cpp";
#ifdef _WIN32
    expected.replace_extension(".obj");
#else
    expected.replace_extension(".o");
#endif
    CHECK(target.obj_files[0].generic_string() == expected.generic_string());
    CHECK(fs::exists(expected));
}

TEST_CASE("build_target adds multiple source files") {
    test_target target("test_build_target_adds_multiple_source_files",
                       "add multiple source files");

    REQUIRE_FALSE(target.find_source_file("other.cpp"));

    target.add_source_files({"a.cpp", "b.cpp"});
    CHECK(target.find_source_file("a.cpp"));
    CHECK(target.find_source_file("b.cpp"));
    CHECK_FALSE(target.find_source_file("c.cpp"));

    target.add_source_files({"c.cpp"});
    CHECK(target.find_source_file("a.cpp"));
    CHECK(target.find_source_file("b.cpp"));
    CHECK(target.find_source_file("c.cpp"));
}

TEST_CASE(
    "build_target removes a named source file and ignores missing paths") {
    test_target target("test_build_target_removes_one_and_ignores_missing",
                       "remove one source file and ignore missing paths");

    target.add_source_files({"a.cpp", "b.cpp"});
    REQUIRE(target.find_source_file("a.cpp"));
    REQUIRE(target.find_source_file("b.cpp"));
    REQUIRE_FALSE(target.find_source_file("missing.cpp"));

    target.remove_source_file("missing.cpp");
    CHECK(target.find_source_file("a.cpp"));
    CHECK(target.find_source_file("b.cpp"));

    target.remove_source_file("a.cpp");
    CHECK_FALSE(target.find_source_file("a.cpp"));
    CHECK(target.find_source_file("b.cpp"));
}

TEST_CASE("build_target removes named source files and ignores missing paths") {
    test_target target("test_build_target_removes_multiple_source_files",
                       "remove multiple source files");

    target.add_source_files({"a.cpp", "b.cpp", "c.cpp"});
    REQUIRE(target.find_source_file("a.cpp"));
    REQUIRE(target.find_source_file("b.cpp"));
    REQUIRE(target.find_source_file("c.cpp"));
    REQUIRE_FALSE(target.find_source_file("missing.cpp"));

    target.remove_source_files({"a.cpp", "missing.cpp", "c.cpp"});

    CHECK_FALSE(target.find_source_file("a.cpp"));
    CHECK(target.find_source_file("b.cpp"));
    CHECK_FALSE(target.find_source_file("c.cpp"));
}

TEST_CASE("build_target removes matching source files and returns the count") {
    test_target target("test_build_target_removes_matching_source_files",
                       "remove matching source files");

    target.add_source_files({"a.cpp", "notes.txt", "b.cpp", "header.h"});
    REQUIRE(target.find_source_file("a.cpp"));
    REQUIRE(target.find_source_file("notes.txt"));
    REQUIRE(target.find_source_file("b.cpp"));
    REQUIRE(target.find_source_file("header.h"));

    const int removed =
        target.remove_source_files([](const std::string& path) -> bool {
            return fs::path(path).extension() == ".cpp";
        });

    CHECK(removed == 2);
    CHECK_FALSE(target.find_source_file("a.cpp"));
    CHECK(target.find_source_file("notes.txt"));
    CHECK_FALSE(target.find_source_file("b.cpp"));
    CHECK(target.find_source_file("header.h"));
}

TEST_CASE("build_target selects files by suffix and recursion setting") {
    const fs::path root = fs::path("build") / "tests" / "unittest" / "work" /
                          "build_target" / "suffix_selection";
    fs::remove_all(root);
    fs::create_directories(root / "nested");

    std::ofstream(root / "main.cpp") << "";
    std::ofstream(root / "notes.txt") << "";
    std::ofstream(root / "nested" / "helper.cpp") << "";
    std::ofstream(root / "nested" / "header.h") << "";

    const std::string main_cpp =
        (root / "main.cpp").lexically_normal().string();
    const std::string notes_txt =
        (root / "notes.txt").lexically_normal().string();
    const std::string helper_cpp =
        (root / "nested" / "helper.cpp").lexically_normal().string();
    const std::string header_h =
        (root / "nested" / "header.h").lexically_normal().string();

    test_target recursive("test_build_target_suffix_recursive",
                          "select source files by suffix recursively");
    recursive.add_source_files({root.string()}, {".cpp"});
    CHECK(recursive.find_source_file(main_cpp));
    CHECK(recursive.find_source_file(helper_cpp));
    CHECK_FALSE(recursive.find_source_file(notes_txt));
    CHECK_FALSE(recursive.find_source_file(header_h));

    test_target flat("test_build_target_suffix_flat",
                     "select source files by suffix without recursion");
    flat.add_source_files({root.string()}, {".cpp"}, false);
    CHECK(flat.find_source_file(main_cpp));
    CHECK_FALSE(flat.find_source_file(helper_cpp));
    CHECK_FALSE(flat.find_source_file(notes_txt));
    CHECK_FALSE(flat.find_source_file(header_h));
}

TEST_CASE("build_target selects files by predicate and recursion setting") {
    const fs::path root = fs::path("build") / "tests" / "unittest" / "work" /
                          "build_target" / "predicate_selection";
    fs::remove_all(root);
    fs::create_directories(root / "nested");

    std::ofstream(root / "keep_main.cpp") << "";
    std::ofstream(root / "skip_main.cpp") << "";
    std::ofstream(root / "nested" / "keep_helper.h") << "";
    std::ofstream(root / "nested" / "skip_helper.cpp") << "";

    const std::string keep_main =
        (root / "keep_main.cpp").lexically_normal().string();
    const std::string skip_main =
        (root / "skip_main.cpp").lexically_normal().string();
    const std::string keep_helper =
        (root / "nested" / "keep_helper.h").lexically_normal().string();
    const std::string skip_helper =
        (root / "nested" / "skip_helper.cpp").lexically_normal().string();
    const auto keep_named_file = [](const std::string& path) -> bool {
        return fs::path(path).filename().string().starts_with("keep_");
    };

    test_target recursive("test_build_target_predicate_recursive",
                          "select source files by predicate recursively");
    recursive.add_source_files({root.string()}, keep_named_file);
    CHECK(recursive.find_source_file(keep_main));
    CHECK(recursive.find_source_file(keep_helper));
    CHECK_FALSE(recursive.find_source_file(skip_main));
    CHECK_FALSE(recursive.find_source_file(skip_helper));

    test_target flat("test_build_target_predicate_flat",
                     "select source files by predicate without recursion");
    flat.add_source_files({root.string()}, keep_named_file, false);
    CHECK(flat.find_source_file(keep_main));
    CHECK_FALSE(flat.find_source_file(keep_helper));
    CHECK_FALSE(flat.find_source_file(skip_main));
    CHECK_FALSE(flat.find_source_file(skip_helper));
}

} // namespace
