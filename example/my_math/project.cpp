#include "ccc/project.h"
#include <string>
#include <vector>
using namespace std;
using namespace ccc;

void init_project(project* self, string cmd, vector<string> args) {
    // Describe the library
    library mathlib("mymath", library_type::static_library, "My Math Library");
    mathlib.add_source_files({"./math_lib/src/my_math.cpp"}); // Add source file
    mathlib.add_header_folder_paths({
        "./math_lib/inc/", // Add header folder path
    });

    // Describe the executable
    execution myexe("myexe", "My Executable to test my math library.");
    myexe.add_source_files({"./math_exe/src/main.cpp"});

    // Add dependency
    // The first true indicates adding the dependency directly to the target,
    // and the second true indicates compiling it if the dependency does not
    // exist.
    myexe.add_dependency(&mathlib, true, true);
    self->add_task(&myexe);
}

void exit_project(project* self, string cmd, vector<string> args) {}

project my_project("Math", init_project, exit_project, "My Math Project");