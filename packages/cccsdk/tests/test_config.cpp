#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ccc/config.h"
#include "doctest.h"

using namespace ccc;

TEST_CASE("config default construction") {
    config cfg;
    CHECK(cfg.thread_num == 0);
    CHECK(cfg.is_print == true);
    CHECK(cfg.compile_flags.empty());
    CHECK(cfg.link_flags.empty());
    CHECK(cfg.header_folder_paths.empty());
    CHECK(cfg.library_folder_paths.empty());
    CHECK(cfg.macros.empty());
    CHECK(cfg.toolchain.is_empty());
}

TEST_CASE("config copy construction") {
    config a;
    a.compile_flags = {"-O2", "-std=c++20"};
    a.is_print = false;
    a.thread_num = 8;

    config b(a);
    CHECK(b.compile_flags == a.compile_flags);
    CHECK(b.is_print == a.is_print);
    CHECK(b.thread_num == a.thread_num);
}

TEST_CASE("config assignment") {
    config a;
    a.compile_flags = {"-O2"};
    a.is_print = false;
    config b;
    b = a;
    CHECK(b.compile_flags == a.compile_flags);
    CHECK(b.is_print == false);
}

TEST_CASE("config_manager default construction") {
    config_manager mgr;
    CHECK(mgr.config.thread_num == 0);
    CHECK(mgr.config.is_print == true);
}

TEST_CASE("config_manager set_config") {
    config_manager mgr;
    config cfg;
    cfg.compile_flags = {"-O2"};
    cfg.is_print = false;
    mgr.set_config(cfg);
    CHECK(mgr.config.compile_flags == cfg.compile_flags);
    CHECK(mgr.config.is_print == false);
}

TEST_CASE("config_manager set_compile_flags") {
    config_manager mgr;
    mgr.set_compile_flags({"-Wall", "-Wextra"});
    CHECK(mgr.config.compile_flags.size() == 2);
    CHECK(mgr.config.compile_flags[0] == "-Wall");
    CHECK(mgr.config.compile_flags[1] == "-Wextra");
}

TEST_CASE("config_manager add_compile_flags") {
    config_manager mgr;
    mgr.set_compile_flags({"-std=c++20"});
    mgr.add_compile_flags({"-Wall", "-Wextra"});
    CHECK(mgr.config.compile_flags.size() == 3);
    CHECK(mgr.config.compile_flags[0] == "-std=c++20");
    CHECK(mgr.config.compile_flags[1] == "-Wall");
    CHECK(mgr.config.compile_flags[2] == "-Wextra");
}

TEST_CASE("config_manager set_link_flags") {
    config_manager mgr;
    mgr.set_link_flags({"-L./lib", "-lmylib"});
    CHECK(mgr.config.link_flags.size() == 2);
    CHECK(mgr.config.link_flags[0] == "-L./lib");
    CHECK(mgr.config.link_flags[1] == "-lmylib");
}

TEST_CASE("config_manager add_link_flags") {
    config_manager mgr;
    mgr.set_link_flags({"-L./lib"});
    mgr.add_link_flags({"-lmylib"});
    CHECK(mgr.config.link_flags.size() == 2);
    CHECK(mgr.config.link_flags[0] == "-L./lib");
    CHECK(mgr.config.link_flags[1] == "-lmylib");
}

TEST_CASE("config_manager set_thread_num") {
    config_manager mgr;
    mgr.set_thread_num(16);
    CHECK(mgr.config.thread_num == 16);
    mgr.set_thread_num(0);
    CHECK(mgr.config.thread_num == 0);
}

TEST_CASE("config_manager set_header_folder_paths") {
    config_manager mgr;
    mgr.set_header_folder_paths({"./include", "./vendor"});
    CHECK(mgr.config.header_folder_paths.size() == 2);
    CHECK(mgr.config.header_folder_paths[0] == "./include");
    CHECK(mgr.config.header_folder_paths[1] == "./vendor");
}

TEST_CASE("config_manager add_header_folder_paths") {
    config_manager mgr;
    mgr.set_header_folder_paths({"./include"});
    mgr.add_header_folder_paths({"./vendor", "./thirdparty"});
    CHECK(mgr.config.header_folder_paths.size() == 3);
    CHECK(mgr.config.header_folder_paths[0] == "./include");
    CHECK(mgr.config.header_folder_paths[1] == "./vendor");
    CHECK(mgr.config.header_folder_paths[2] == "./thirdparty");
}

TEST_CASE("config_manager set_macros") {
    config_manager mgr;
    mgr.set_marcos({"DEBUG", "VERSION=1"});
    CHECK(mgr.config.macros.size() == 2);
    CHECK(mgr.config.macros[0] == "DEBUG");
    CHECK(mgr.config.macros[1] == "VERSION=1");
}

TEST_CASE("config_manager add_macros") {
    config_manager mgr;
    mgr.set_marcos({"DEBUG"});
    mgr.add_marcos({"VERSION=1", "_WIN32"});
    CHECK(mgr.config.macros.size() == 3);
    CHECK(mgr.config.macros[0] == "DEBUG");
    CHECK(mgr.config.macros[1] == "VERSION=1");
    CHECK(mgr.config.macros[2] == "_WIN32");
}

TEST_CASE("config_manager from config") {
    config cfg;
    cfg.compile_flags = {"-O2"};
    cfg.thread_num = 4;
    config_manager mgr(cfg);
    CHECK(mgr.config.compile_flags.size() == 1);
    CHECK(mgr.config.compile_flags[0] == "-O2");
    CHECK(mgr.config.thread_num == 4);
}
