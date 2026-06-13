#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/execution.h"
#include "ccc/library.h"
#include "ccc/toolchain.h"
#include "doctest.h"

#include <filesystem>
#include <string>
#include <vector>

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

bool contains(const std::vector<std::string>& values,
              const std::string& expected) {
    for (const auto& value : values) {
        if (value == expected)
            return true;
    }
    return false;
}
} // namespace

TEST_CASE("library init applies platform names for static libraries") {
    config windows_cfg;
    windows_cfg.toolchain = built_in_toolchain::gnu_toolchain(windows_os, cpp);
    library windows_lib("mymath", static_library, "math");
    windows_lib.init(windows_cfg);
    CHECK(windows_lib.name == "libmymath.lib");

    config linux_cfg;
    linux_cfg.toolchain = built_in_toolchain::gnu_toolchain(linux_os, cpp);
    library linux_lib("mymath", static_library, "math");
    linux_lib.init(linux_cfg);
    CHECK(linux_lib.name == "libmymath.a");
}

TEST_CASE("library transmit passes static library output to parent objects") {
    execution consumer("__test_static_consumer", "consumer");
    library dep("__test_static_dep", static_library, "dependency");
    dep.output_path = "build/tests/unittest/work/library";

    dep.transmit(consumer);

    REQUIRE(consumer.obj_files.size() == 1);
    CHECK(consumer.obj_files[0].find("__test_static_dep") != std::string::npos);
}

TEST_CASE("library transmit passes shared library search data to parent") {
    config cfg;
    cfg.toolchain = built_in_toolchain::gnu_toolchain(linux_os, cpp);

    execution consumer("__test_shared_consumer", "consumer");
    library dep("sharedname", shared_library, "dependency");
    dep.output_path = "build/tests/unittest/work/library";
    dep.init(cfg);

    dep.transmit(consumer);

    CHECK(contains(consumer.config.library_folder_paths,
                   "build/tests/unittest/work/library"));
    CHECK(contains(consumer.lib_files, "sharedname"));
}

TEST_CASE("library link records command failure in status") {
    config cfg;
    cfg.is_print = false;
    cfg.toolchain = built_in_toolchain::gnu_toolchain();

    fs::path out_dir =
        fs::path("build") / "tests" / "unittest" / "work" / "library";
    fs::create_directories(out_dir);

    library lib("__test_library_link_failure", static_library, "link failure");
    lib.output_path = out_dir.string();
    lib.init(cfg);
    lib.config.toolchain.link_format = Format(quiet_failure_command());

    lib.link(cfg);

    REQUIRE_FALSE(lib.status.empty());
    CHECK(lib.status[0].find("Fail to link") != std::string::npos);
}
