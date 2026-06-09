#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#include "ccc/compile_task.h"

namespace ccc {
enum library_type { static_library, shared_library };

class library : public ccc::compile_task {
  public:
    /**
     * @brief Construct a new library object.
     *
     * @param name The name of the library.
     * @param type The type of the library.
     * @param description The description of the library.
     * @param loc The location of the library.
     */
    library(std::string name, ccc::library_type type, std::string description,
            std::source_location loc = std::source_location::current());

    /**
     * @brief Construct a new library object by copying another library object.
     *
     * @param other The library object to be copied.
     */
    library(const library& other)
        : ccc::compile_task(other), type(other.type) {}

    /**
     * @brief Add information to the parent task based on different library
     * types.
     *
     * @param super The parent task.
     */
    void transmit(ccc::compile_task& super) override;

    library_type type = library_type::static_library;

    /**
     * @brief Initialize library.
     *
     * @param project_cfg The configuration of the project.
     */
    void init(const ccc::config& project_cfg) override;

    /**
     * @brief Rewrite the link method of the compile_task class, call the
     *        compile method in it, and link the intermediate products.
     *
     * @param project_cfg The configuration of the project.
     */
    void link(const ccc::config& project_cfg) override;
};
} // namespace ccc

#endif