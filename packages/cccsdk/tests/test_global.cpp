#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/command.h"
#include "ccc/global.h"
#include "doctest.h"

#include <string>
#include <vector>

using namespace ccc;

namespace {
bool global_test_func_called = false;
void global_test_func(std::vector<std::string>) {
    global_test_func_called = true;
}

bool gvar_low_called = false;
void gvar_low_func(std::vector<std::string>) { gvar_low_called = true; }

bool gvar_high_called = false;
void gvar_high_func(std::vector<std::string>) { gvar_high_called = true; }
} // namespace

TEST_CASE("global_var add_desc and get_descs") {
    global_var::add_desc("__test_desc1", "description a",
                         std::source_location::current());
    global_var::add_desc("__test_desc1", "description b",
                         std::source_location::current());

    auto descs = global_var::get_descs("__test_desc1");
    CHECK(descs.size() >= 2);
}

TEST_CASE("global_var get_descs non-existent returns empty") {
    auto descs = global_var::get_descs("__nonexistent_desc");
    CHECK(descs.empty());
}

TEST_CASE("global_var add_cmd and get_cmd") {
    command cmd("__test_gvar_cmd", global_test_func, "global test command",
                command::priority::normal);

    auto* found = global_var::get_cmd("__test_gvar_cmd");
    CHECK(found != nullptr);
}

TEST_CASE("global_var get_cmd prioritizes high over normal") {
    gvar_low_called = false;
    gvar_high_called = false;

    command cmd_low("__test_gvar_prio", gvar_low_func, "low",
                    command::priority::low);
    command cmd_high("__test_gvar_prio", gvar_high_func, "high",
                     command::priority::high);

    auto* found = global_var::get_cmd("__test_gvar_prio");
    found->run({});
    CHECK(gvar_high_called);
    CHECK_FALSE(gvar_low_called);
}

TEST_CASE("global_var get_cmd returns correct run function") {
    global_test_func_called = false;

    command cmd("__test_gvar_run", global_test_func, "run test",
                command::priority::normal);
    (void)cmd;

    auto* found = global_var::get_cmd("__test_gvar_run");
    found->run({});
    CHECK(global_test_func_called);
}

TEST_CASE("global_var add_desc single name") {
    global_var::add_desc("__test_single_desc", "single description",
                         std::source_location::current());
    auto descs = global_var::get_descs("__test_single_desc");
    CHECK(descs.size() >= 1);
}
