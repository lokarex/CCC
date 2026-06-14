#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "support/example_harness.hpp"

using namespace ccc_system_test;

TEST_CASE("ccc builds hello_world example through public project header") {
    Example example("hello_world");

    REQUIRE(example.build());
    CHECK(example.has_executable("hello_world"));
    CHECK(example.run());
}
