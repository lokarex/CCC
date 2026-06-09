#ifndef __DEPENDENCY_H__
#define __DEPENDENCY_H__

namespace ccc {
class dependency_description {
  public:
    /* Whether add it directly to the compile task. */
    bool is_transmit;
    /* Whether to actively compile dependencies, if dependencies do not exist.
     */
    bool is_compile;

    /**
     * @brief Construct a new dependency description object.
     *
     * @param is_transmit Whether add it directly to the compile task.
     * @param is_compile Whether to actively compile dependencies, if
     *                   dependencies do not exist.
     */
    dependency_description(bool is_transmit = true, bool is_compile = false) {
        this->is_transmit = is_transmit;
        this->is_compile = is_compile;
    }
};
} // namespace ccc

#endif