#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "support/example_harness.hpp"

using namespace ccc_system_test;

TEST_CASE("ccc builds and runs my_math dependency example") {
    Example example("my_math");

    REQUIRE(example.build());
    REQUIRE(example.has_executable("myexe"));
    CHECK(example.run("myexe"));
}
