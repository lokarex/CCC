#ifndef __EXECUTION_H__
#define __EXECUTION_H__

#include "ccc/compile_task.h"

namespace ccc {
class execution : public ccc::compile_task {
  public:
    /**
     * @brief Construct a new execution object.
     *
     * @param name The name of the execution task.
     * @param description The description of the execution task.
     * @param loc The location of the execution.
     */
    execution(std::string name, std::string description,
              std::source_location loc = std::source_location::current());

    /**
     * @brief Construct a new execution object by copy.
     *
     * @param other The execution task to be copied.
     */
    execution(const execution& other) : ccc::compile_task(other) {}

    /**
     * @brief Initialize execution.
     *
     * @param project_cfg The project configuration.
     */
    void init(const ccc::config& project_cfg) override;

    /**
     * @brief Rewrite the link method of the compile_task class, call the
     *        compile method in it, and link the intermediate products.
     *
     * @param project_cfg The configuration of the project.
     */
    void link(const ccc::config& project_cfg) override;

    /**
     * @brief Execution does not anything in the transmit method.
     *
     * @param super The parent task.
     */
    void transmit(ccc::compile_task& super) override;
};
} // namespace ccc

#endif