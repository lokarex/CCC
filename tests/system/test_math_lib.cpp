#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "support/example_harness.hpp"

using namespace ccc_system_test;

TEST_CASE("ccc builds math_lib example through public project header") {
    Example example("math_lib");

    REQUIRE(example.build());
    CHECK(example.has_static_library("mymath"));
}
