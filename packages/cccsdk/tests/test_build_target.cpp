#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"

#include "ccc/build_target.h"
#include "ccc/config.h"

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
