#ifndef __CCC_PROJECT_H__
#define __CCC_PROJECT_H__

#include "ccc/compile_task.h"
#include "ccc/config.h"
#include "ccc/execution.h"
#include "ccc/library.h"

#include <memory>
#include <source_location>
#include <vector>

namespace ccc {

class third_party {
  public:
    /* The name of the project. */
    std::string name;

    /* The description of the project. */
    std::string description;

    /* The stored path of the project. */
    std::string path;

    /* The defined location of the project */
    std::source_location loc;

    /* The simple information of the location. */
    std::string loc_info;

    /* The third_party of the project. */
    std::vector<ccc::third_party*> third_part;

    third_party(std::string name, std::string description, bool ccc_manage,
                std::string path, std::source_location loc);

    virtual bool process(std::vector<std::string> args) = 0;
};

class project : public ccc::third_party, public ccc::config_manager {
  public:
    /**
     * @brief Construct a new project object.
     *
     * @param name The name of the project.
     * @param init_func The init_func is executed before processing the project.
     * @param exit_func The exit_func is executed after processing the project.
     * @param description The description of the project.
     * @param path The path of the project.
     * @param loc The defined location of the project.
     */
    project(
        std::string name,
        auto (*init_func)(project*, std::string, std::vector<std::string>)
            ->void,
        auto (*exit_func)(project*, std::string, std::vector<std::string>)
            ->void,
        std::string description,
        std::string path = []() -> std::string {
            FILE* pipe = popen("pwd", "r");
            if (!pipe)
                return "";
            char buffer[128];
            std::string result;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            pclose(pipe);
            if (!result.empty() && result.back() == '\n') {
                result.pop_back();
            }
            return result;
        }(),
        std::source_location loc = std::source_location::current());

    /* The init_func is executed before processing the project. */
    void (*init_func)(project*, std::string, std::vector<std::string>);
    /* The exit_func is executed after processing the project. */
    void (*exit_func)(project*, std::string, std::vector<std::string>);

    /* Used to pass parameters between init_func and exit_func. */
    void* arg;

    std::vector<ccc::compile_task*> tasks;

    /* Process the project.  */
    bool process(std::vector<std::string> args);

    /* The convenient utility functions provided by ccc. */

    /**
     * @brief Add a task need to be compile to the project.
     *
     * @param task The task need to be added to the project.
     */
    void add_task(ccc::compile_task* task);

  private:
};

} // namespace ccc

#endif