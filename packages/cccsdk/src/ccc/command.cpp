#include "ccc/command.h"

#include "ccc/global.h"

ccc::command::command(std::string name,
                      auto (*run)(std::vector<std::string> args)->void,
                      std::string description, ccc::command::priority prio,
                      std::source_location loc)
    : run(run) {
    // Add description.
    ccc::global_var::add_desc(name, description, loc);

    // Register the command.
    ccc::global_var::add_cmd(name, this, prio);

    // Obtain location.
    this->loc = loc;
}

ccc::command::command(std::initializer_list<std::string> names,
                      auto (*run)(std::vector<std::string> args)->void,
                      std::string description, ccc::command::priority prio,
                      std::source_location loc)
    : run(run) {
    // Register the command.
    for (auto name : names) {
        // Add description.
        ccc::global_var::add_desc(name, description, loc);

        // Register the command.
        ccc::global_var::add_cmd(name, this, prio);
    }

    // Obtain location.
    this->loc = loc;
}

namespace ccc {
std::string operator+(command::priority prio, const std::string& str) {
    switch (prio) {
    case command::priority::low:
        return "low" + str;
    case command::priority::normal:
        return "normal" + str;
    case command::priority::high:
        return "high" + str;
    default:
        return "unknown priority" + str;
    }
}

std::string operator+(const std::string& str, command::priority prio) {
    switch (prio) {
    case command::priority::low:
        return str + "low";
    case command::priority::normal:
        return str + "normal";
    case command::priority::high:
        return str + "high";
    default:
        return str + "unknown priority";
    }
}
} // namespace ccc