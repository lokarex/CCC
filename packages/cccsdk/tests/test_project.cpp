#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/execution.h"
#include "ccc/project.h"
#include "doctest.h"

#include <filesystem>
#include <string>
#include <vector>

using namespace ccc;
namespace fs = std::filesystem;

namespace {
std::vector<std::string> lifecycle_events;

void record_init(project*, std::string cmd, std::vector<std::string> args) {
    lifecycle_events.push_back("init:" + cmd + ":" +
                               std::to_string(args.size()));
}

void record_exit(project*, std::string cmd, std::vector<std::string> args) {
    lifecycle_events.push_back("exit:" + cmd + ":" +
                               std::to_string(args.size()));
}

void noop(project*, std::string, std::vector<std::string>) {}
} // namespace

TEST_CASE("project default path is current directory") {
    project p("__test_project_path", noop, noop, "path test");

    CHECK(fs::path(p.path).lexically_normal() ==
          fs::current_path().lexically_normal());
}

TEST_CASE("project process calls init then exit around task processing") {
    lifecycle_events.clear();
    project p("__test_project_lifecycle", record_init, record_exit,
              "lifecycle test", fs::current_path().string());

    bool ok = p.process({"alpha", "beta"});

    CHECK(ok);
    REQUIRE(lifecycle_events.size() == 2);
    CHECK(lifecycle_events[0] == "init:build:2");
    CHECK(lifecycle_events[1] == "exit:build:2");
}

TEST_CASE("project add_task stores task for processing") {
    project p("__test_project_add_task", noop, noop, "add task test",
              fs::current_path().string());
    execution exe("__test_project_task", "task");

    p.add_task(&exe);

    REQUIRE(p.tasks.size() == 1);
    CHECK(p.tasks[0] == &exe);
}
