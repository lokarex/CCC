#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "support/example_harness.hpp"

#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;
using namespace ccc_system_test;

TEST_CASE("ccc builds and runs my_math dependency example") {
    REQUIRE(fs::exists(ccc_executable()));

    fs::path project_dir = copy_example("my_math");
    CHECK(std::system(ccc_project_build_command(project_dir).c_str()) == 0);

    fs::path exe = project_dir / "build" / "bin" / exe_name("myexe");
    REQUIRE(fs::exists(exe));
    CHECK(std::system(run_command(exe).c_str()) == 0);
}
