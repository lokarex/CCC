#ifndef __CCC_THIRD_PARTY_HPP__
#define __CCC_THIRD_PARTY_HPP__

#include "ccc/project.h"
#include <vector>

namespace ccc {

class github_repository {
    std::string clone_url;
    std::string branch;
    std::string user_name;
    std::string repository_name;

  public:
    bool download() { return true; }
    bool switch_branch() { return true; }
};

class makefile_project : public ccc::third_party {
  public:
    makefile_project(ccc::github_repository repo);

    bool process(std::vector<std::string>) override { return true; }
};

class cmake_project : public ccc::third_party {
  public:
    cmake_project(ccc::github_repository repo);

    bool process(std::vector<std::string>) override { return true; }
};
} // namespace ccc

#endif