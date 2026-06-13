#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/execution.h"
#include "ccc/toolchain.h"
#include "doctest.h"

#include <filesystem>
#include <string>

using namespace ccc;
namespace fs = std::filesystem;

namespace {
std::string quiet_failure_command() {
#ifdef _WIN32
    return "cmd /C exit /B 7";
#else
    return "sh -c 'exit 7'";
#endif
}
} // namespace

TEST_CASE("execution initializes output path and platform suffix") {
    config cfg;
    cfg.toolchain = built_in_toolchain::gnu_toolchain(windows_os, cpp);

    execution exe("sample", "sample executable");
    exe.init(cfg);

    CHECK(exe.output_path == "./build/bin");
    CHECK(exe.name == "sample.exe");
}

TEST_CASE("execution link records command failure in status") {
    config cfg;
    cfg.is_print = false;
    cfg.toolchain = built_in_toolchain::gnu_toolchain();

    fs::path out_dir =
        fs::path("build") / "tests" / "unittest" / "work" / "execution";
    fs::create_directories(out_dir);

    execution exe("__test_execution_link_failure", "link failure");
    exe.output_path = out_dir.string();
    exe.init(cfg);
    exe.config.toolchain.link_format = Format(quiet_failure_command());

    exe.link(cfg);

    REQUIRE_FALSE(exe.status.empty());
    CHECK(exe.status[0].find("Fail to link") != std::string::npos);
}
