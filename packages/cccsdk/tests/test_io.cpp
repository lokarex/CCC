#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "util/io.h"

#include <string>

namespace {
std::string success_command() {
#ifdef _WIN32
    return "cmd /C exit /B 0";
#else
    return "sh -c 'exit 0'";
#endif
}

std::string failure_command() {
#ifdef _WIN32
    return "cmd /C exit /B 7";
#else
    return "sh -c 'exit 7'";
#endif
}
} // namespace

TEST_CASE("exec_command returns true for successful commands") {
    CHECK(ccc::io::exec_command(success_command(), false, false));
}

TEST_CASE("exec_command returns false for failed commands") {
    CHECK_FALSE(ccc::io::exec_command(failure_command(), false, false));
}
