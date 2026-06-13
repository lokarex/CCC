#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "support/example_harness.hpp"

#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;
using namespace ccc_system_test;

TEST_CASE("ccc builds hello_world example through public project header") {
    REQUIRE(fs::exists(ccc_executable()));

    fs::path project_dir = copy_example("hello_world");
    CHECK(std::system(ccc_project_build_command(project_dir).c_str()) == 0);
    CHECK(fs::exists(project_dir / "build" / "bin" / exe_name("hello_world")));
}
