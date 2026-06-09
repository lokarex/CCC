#include "ccc/command.h"
#include "ccc/global.h"
#include "ccc/info.hpp"
#include "util/io.h"

#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    // If there is no argument, print the help message.
    if (argc == 1) {
        ccc::io::print(ccc::info::help_msg);
        return -1;
    }

    std::string target_cmd = "";
    // Determine the target command
    target_cmd = argv[1];

    // Get the arguments
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++)
        args.push_back(argv[i]);

    // Run the target command
    ccc::global_var::get_cmd(target_cmd)->run(args);
    return 0;
}