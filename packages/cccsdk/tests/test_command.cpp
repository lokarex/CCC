#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/command.h"
#include "ccc/global.h"
#include "doctest.h"
#include <string>
#include <vector>

using namespace ccc;

namespace {
bool test_func_called = false;
void test_func(std::vector<std::string>) { test_func_called = true; }

bool high_prio_called = false;
void high_prio_func(std::vector<std::string>) { high_prio_called = true; }

bool multi_func_called = false;
void multi_func(std::vector<std::string>) { multi_func_called = true; }
} // namespace

TEST_CASE("command priority enum values") {
    CHECK(static_cast<int>(command::priority::low) == 0);
    CHECK(static_cast<int>(command::priority::normal) == 1);
    CHECK(static_cast<int>(command::priority::high) == 2);
}

TEST_CASE("command priority operator+ priority + string") {
    CHECK(command::priority::low + std::string("_test") == "low_test");
    CHECK(command::priority::normal + std::string("_test") == "normal_test");
    CHECK(command::priority::high + std::string("_test") == "high_test");
}

TEST_CASE("command priority operator+ string + priority") {
    CHECK(std::string("test_") + command::priority::low == "test_low");
    CHECK(std::string("test_") + command::priority::normal == "test_normal");
    CHECK(std::string("test_") + command::priority::high == "test_high");
}

TEST_CASE("command registers in global_var") {
    test_func_called = false;

    command cmd("__test_cmd_reg", test_func, "test command registration",
                command::priority::normal);

    auto* found = global_var::get_cmd("__test_cmd_reg");
    CHECK(found != nullptr);
    CHECK(found->run == test_func);

    found->run({});
    CHECK(test_func_called);
}

TEST_CASE("command with high priority overrides normal") {
    high_prio_called = false;

    command cmd1("__test_cmd_prio1", test_func, "normal priority",
                 command::priority::normal);
    command cmd2("__test_cmd_prio1", high_prio_func, "high priority",
                 command::priority::high);

    auto* found = global_var::get_cmd("__test_cmd_prio1");
    CHECK(found != nullptr);

    found->run({});
    CHECK(high_prio_called);
}

TEST_CASE("command multiple names registration") {
    multi_func_called = false;

    command cmd({"__test_cmd_multi1", "__test_cmd_multi2"}, multi_func,
                "multi-name command");

    auto* found1 = global_var::get_cmd("__test_cmd_multi1");
    CHECK(found1 != nullptr);

    auto* found2 = global_var::get_cmd("__test_cmd_multi2");
    CHECK(found2 != nullptr);

    found1->run({});
    CHECK(multi_func_called);
}

TEST_CASE("command default priority is normal") {
    command cmd("__test_cmd_default", test_func, "test default priority");
    auto* found = global_var::get_cmd("__test_cmd_default");
    CHECK(found != nullptr);
}
