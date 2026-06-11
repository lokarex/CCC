#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/toolchain.h"
#include "doctest.h"

#include <string>
#include <unordered_map>
#include <vector>

using namespace ccc;

TEST_CASE("Format replace simple variable") {
    Format fmt("$(COMPILER) $(SOURCE_FILE)");
    std::unordered_map<std::string, std::vector<std::string>> mp = {
        {"COMPILER", {"g++"}},
        {"SOURCE_FILE", {"main.cpp"}},
    };
    std::string result = fmt.replace(mp);
    CHECK(result == "g++ main.cpp");
}

TEST_CASE("Format replace with brace blocks") {
    Format fmt("$(COMPILER) {-I$(HEADER_FOLDERS)}");
    std::unordered_map<std::string, std::vector<std::string>> mp = {
        {"COMPILER", {"g++"}},
        {"HEADER_FOLDERS", {"./include", "./vendor"}},
    };
    std::string result = fmt.replace(mp);
    CHECK(result == "g++ -I./include -I./vendor");
}

TEST_CASE("Format replace brace block with single value") {
    Format fmt("{-D$(MACROS)}");
    std::unordered_map<std::string, std::vector<std::string>> mp = {
        {"MACROS", {"DEBUG"}},
    };
    std::string result = fmt.replace(mp);
    CHECK(result == "-DDEBUG");
}

TEST_CASE("Format replace brace block with prefix/suffix") {
    Format fmt("$(LINKER) {-L$(LIBRARY_FOLDERS)}");
    std::unordered_map<std::string, std::vector<std::string>> mp = {
        {"LINKER", {"g++"}},
        {"LIBRARY_FOLDERS", {"./lib", "/usr/lib"}},
    };
    std::string result = fmt.replace(mp);
    CHECK(result == "g++ -L./lib -L/usr/lib");
}

TEST_CASE("Format replace multiple brace blocks") {
    Format fmt("{-I$(HEADER_FOLDERS)} {-L$(LIBRARY_FOLDERS)}");
    std::unordered_map<std::string, std::vector<std::string>> mp = {
        {"HEADER_FOLDERS", {"./inc"}},
        {"LIBRARY_FOLDERS", {"./lib"}},
    };
    std::string result = fmt.replace(mp);
    CHECK(result == "-I./inc -L./lib");
}

TEST_CASE("Format replace missing variable preserves original") {
    Format fmt("$(MISSING)");
    std::unordered_map<std::string, std::vector<std::string>> mp = {};
    std::string result = fmt.replace(mp);
    CHECK(result == "$(MISSING)");
}

TEST_CASE("toolchain default construction is empty") {
    toolchain tc;
    CHECK(tc.is_empty());
    CHECK(tc.name.empty());
    CHECK(tc.compiler.empty());
    CHECK(tc.linker.empty());
}

TEST_CASE("toolchain parameterized construction") {
    toolchain tc(windows_os, cpp, "test", "g++", "g++", Format("compile"),
                 Format("exec_compile"), Format("static_compile"),
                 Format("shared_compile"), Format("link"), Format("exec_link"),
                 Format("static_link"), Format("shared_link"));
    CHECK_FALSE(tc.is_empty());
    CHECK(tc.name == "test");
    CHECK(tc.compiler == "g++");
    CHECK(tc.linker == "g++");
    CHECK(tc.target_os == windows_os);
    CHECK(tc.language == cpp);
    CHECK(tc.compile_format.format == "compile");
    CHECK(tc.execution_compile_format.format == "exec_compile");
    CHECK(tc.static_library_compile_format.format == "static_compile");
    CHECK(tc.shared_library_compile_format.format == "shared_compile");
    CHECK(tc.link_format.format == "link");
    CHECK(tc.execution_link_format.format == "exec_link");
    CHECK(tc.static_library_link_format.format == "static_link");
    CHECK(tc.shared_library_link_format.format == "shared_link");
}

TEST_CASE("toolchain equality and inequality") {
    toolchain tc1(windows_os, cpp, "test", "g++", "g++", Format(), Format(),
                  Format(), Format(), Format(), Format(), Format(), Format());
    toolchain tc2(windows_os, cpp, "test", "g++", "g++", Format(), Format(),
                  Format(), Format(), Format(), Format(), Format(), Format());
    toolchain tc3(linux_os, cpp, "test", "g++", "g++", Format(), Format(),
                  Format(), Format(), Format(), Format(), Format(), Format());

    CHECK(tc1 == tc2);
    CHECK(tc1 != tc3);
}

TEST_CASE("built_in_toolchain gnu cpp defaults") {
    auto tc = built_in_toolchain::gnu_toolchain();
    CHECK_FALSE(tc.is_empty());
    CHECK(tc.name == "gnu");
    CHECK(tc.compiler == "g++");
    CHECK(tc.linker == "g++");
    CHECK(tc.language == cpp);
}

TEST_CASE("built_in_toolchain gnu c language") {
    auto tc = built_in_toolchain::gnu_toolchain(language_type::c);
    CHECK(tc.name == "gnu");
    CHECK(tc.compiler == "gcc");
    CHECK(tc.linker == "gcc");
    CHECK(tc.language == c);
}

TEST_CASE("built_in_toolchain clang cpp defaults") {
    auto tc = built_in_toolchain::clang_toolchain();
    CHECK_FALSE(tc.is_empty());
    CHECK(tc.name == "clang");
    CHECK(tc.compiler == "clang++");
    CHECK(tc.linker == "clang++");
}

TEST_CASE("built_in_toolchain clang c language") {
    auto tc = built_in_toolchain::clang_toolchain(language_type::c);
    CHECK(tc.name == "clang");
    CHECK(tc.compiler == "clang");
    CHECK(tc.linker == "clang");
    CHECK(tc.language == c);
}

TEST_CASE("built_in_toolchain msvc cpp") {
    auto tc = built_in_toolchain::msvc_toolchain(windows_os);
    CHECK(tc.name == "msvc");
    CHECK(tc.compiler.empty());
    CHECK(tc.linker.empty());
    CHECK(tc.target_os == windows_os);
}

TEST_CASE("built_in_toolchain gnu has valid formats") {
    auto tc = built_in_toolchain::gnu_toolchain();
    CHECK_FALSE(tc.execution_compile_format.format.empty());
    CHECK_FALSE(tc.execution_link_format.format.empty());
    CHECK_FALSE(tc.static_library_link_format.format.empty());
    CHECK_FALSE(tc.shared_library_link_format.format.empty());
}

TEST_CASE("Format replace with end-to-end compile command") {
    auto tc = built_in_toolchain::gnu_toolchain();
    Format fmt(tc.execution_compile_format.format);

    std::unordered_map<std::string, std::vector<std::string>> mp = {
        {"COMPILER", {"g++"}},
        {"SOURCE_FILE", {"test.cpp"}},
        {"OBJECT_FILE", {"build/obj/test.obj"}},
        {"COMPILE_FLAGS", {"-std=c++20", "-g"}},
        {"HEADER_FOLDERS", {}},
        {"MACROS", {}},
    };
    std::string result = fmt.replace(mp);
    CHECK(result.find("g++") != std::string::npos);
    CHECK(result.find("test.cpp") != std::string::npos);
    CHECK(result.find("test.obj") != std::string::npos);
    CHECK(result.find("-std=c++20") != std::string::npos);
    CHECK(result.find("-g") != std::string::npos);
}

TEST_CASE("built_in_toolchain gnu with specific target_os") {
    auto tc = built_in_toolchain::gnu_toolchain(linux_os, cpp);
    CHECK(tc.target_os == linux_os);
    CHECK(tc.name == "gnu");
}
