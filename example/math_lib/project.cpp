#include "ccc/project.h"
#include <string>
#include <vector>
using namespace std;
using namespace ccc;

void init_project(project* self, string cmd, vector<string> args) {
    /* Create a library, When the suffix of 'name' is not written, ccc will
     * automatically add the prefix and suffix of the library based on the
     * platform. For example, under the Windows operating system, the name of a
     * static library will be changed to lib<name>.lib */
    library mathlib("mymath", library_type::static_library, "My Math Library");

    /* Change to shared library(Default is static library). */
    // mathlib.type = shared_library;

    mathlib.add_source_files({"./src/my_math.cpp"}); // Add source file
    mathlib.add_header_folder_paths({
        "./inc/", // Add header folder path
    });
    self->add_task(&mathlib); // Add library to project
}

void exit_project(project* self, string cmd, vector<string> args) {}

project my_project("Math_Lib", init_project, exit_project, "My Math Project");