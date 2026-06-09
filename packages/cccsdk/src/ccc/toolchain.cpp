#include "ccc/toolchain.h"

#include <iostream>
#include <regex>

namespace ccc {
std::string
Format::replace(std::unordered_map<std::string, std::vector<std::string>>& mp) {
    std::string ret = format;

    // First, match all templates that are wrapped by {}
    std::regex brace_pattern(R"(\{(.*?)\})");
    std::smatch brace_match;

    while (std::regex_search(ret, brace_match, brace_pattern)) {
        std::string segment = brace_match[1];
        std::string replacement;

        // Separate prefixe, variable name, and suffixe
        std::regex var_pattern(R"((.*?)\$\((\w+)\)(.*))");
        std::smatch var_match;

        if (std::regex_match(segment, var_match, var_pattern)) {
            std::string prefix = var_match[1];
            std::string key = var_match[2];
            std::string suffix = var_match[3];

            if (mp.count(key)) {
                for (const auto& val : mp[key]) {
                    if (!replacement.empty())
                        replacement += " "; // Maintain space separation
                                            // between parameters
                    replacement += prefix + val + suffix;
                }
            }
        }

        ret.replace(brace_match.position(), brace_match.length(), replacement);
    }

    // Handling ordinary variable replacement
    for (auto& [key, values] : mp) {
        std::string replacement = "";
        if (!values.empty()) {
            for (const auto& val : values) {
                if (!replacement.empty())
                    replacement += " ";
                replacement += val;
            }
        }
        ret = std::regex_replace(ret, std::regex(R"(\$\(()" + key + R"()\))"),
                                 replacement);
    }

    return ret;
}

namespace built_in_toolchain {

ccc::toolchain gnu_toolchain(ccc::system_type target_os,
                             ccc::language_type language) {
    // Check target OS
    if (target_os != system_type::windows_os &&
        target_os != system_type::linux_os) {
        std::cerr << "Error: Unsupported target OS for gnu toolchain."
                  << std::endl;
        exit(1);
    }

    ccc::toolchain tc = ccc::toolchain(
        target_os, language, "gnu",

        "", "",

        ccc::Format(""),
        ccc::Format("$(COMPILER) -c $(SOURCE_FILE) -o "
                    "$(OBJECT_FILE) $(COMPILE_FLAGS) {-I$(HEADER_FOLDERS)} "
                    "{-D$(MACROS)} -fdiagnostics-color=always 2>&1"),
        ccc::Format("$(COMPILER) -c $(SOURCE_FILE) -o "
                    "$(OBJECT_FILE) $(COMPILE_FLAGS) {-I$(HEADER_FOLDERS)} "
                    "{-D$(MACROS)} -fdiagnostics-color=always 2>&1"),
        ccc::Format(
            "$(COMPILER) -c $(SOURCE_FILE) -o "
            "$(OBJECT_FILE) $(COMPILE_FLAGS) -fPIC {-I$(HEADER_FOLDERS)} "
            "{-D$(MACROS)} -fdiagnostics-color=always 2>&1"),

        ccc::Format(""),
        ccc::Format(
            "$(LINKER) -o $(OUTPUT_FILE) {$(OBJECT_FILES)} "
            "{-l$(LIBRARY_FILES)} {-L$(LIBRARY_FOLDERS)} {$(LINK_FLAGS)} "
            "-fdiagnostics-color=always 2>&1"),
        ccc::Format("ar rcs $(OUTPUT_FILE) $(OBJECT_FILES)"),
        ccc::Format("$(LINKER) -o $(OUTPUT_FILE) {-l$(LIBRARY_FILES)} "
                    "{-L$(LIBRARY_FOLDERS)} {$(OBJECT_FILES)} "
                    "{$(LINK_FLAGS)} -shared "
                    "-fdiagnostics-color=always 2>&1"));

    // Set compiler and linker based on language type
    if (language == ccc::language_type::cpp) {
        tc.compiler = "g++";
        tc.linker = "g++";
    } else if (language == ccc::language_type::c) {
        tc.compiler = "gcc";
        tc.linker = "gcc";
    } else {
        std::cerr << "Error: Unsupported language type." << std::endl;
        exit(1);
    }

    return tc;
}

ccc::toolchain clang_toolchain(ccc::system_type target_os,
                               ccc::language_type language) {
    // Check target OS
    if (target_os != system_type::windows_os &&
        target_os != system_type::linux_os) {
        std::cerr << "Error: Unsupported target OS for gnu toolchain."
                  << std::endl;
        exit(1);
    }

    auto tc = built_in_toolchain::gnu_toolchain(target_os, language);
    tc.name = "clang";
    tc.static_library_link_format =
        ccc::Format("llvm-ar rcs $(OUTPUT_FILE) $(OBJECT_FILES)");

    if (target_os == ccc::windows_os) {
        // Clang on Windows does not support the - fPIC option.
        tc.shared_library_compile_format = tc.static_library_compile_format;
    }

    // Set compiler and linker based on language type
    if (language == language_type::cpp) {
        tc.compiler = "clang++";
        tc.linker = "clang++";
    } else if (language == language_type::c) {
        tc.compiler = "clang";
        tc.linker = "clang";
    } else {
        std::cerr << "Error: Unsupported language type." << std::endl;
        exit(1);
    }

    return tc;
};

ccc::toolchain msvc_toolchain(ccc::system_type target_os,
                              ccc::language_type language) {
    // Check target OS
    if (target_os != system_type::windows_os) {
        std::cerr << "Error: The MSVC toolchain only supports use under the "
                     "Windows operating system."
                  << std::endl;
        exit(1);
    }

    ccc::toolchain tc = ccc::toolchain(
        target_os, language, "msvc",

        "", "",

        ccc::Format(""), // compile_format
        ccc::Format("cl /c $(SOURCE_FILE) /Fo$(OBJECT_FILE) "
                    "$(COMPILE_FLAGS) {/I$(HEADER_FOLDERS)} "
                    "{/D{MACROS}"), // execution_compile_format
        ccc::Format("cl /c $(SOURCE_FILE) /Fo$(OBJECT_FILE) "
                    "$(COMPILE_FLAGS) {/I$(HEADER_FOLDERS)} "
                    "{/D{MACROS}"), // static_library_compile_format
        ccc::Format("cl /c $(SOURCE_FILE) /Fo$(OBJECT_FILE) "
                    "$(COMPILE_FLAGS) {/I$(HEADER_FOLDERS)} "
                    "{/D{MACROS}"), // shared_library_compile_format

        ccc::Format(""), // link_format
        ccc::Format("cl /Fe$(OUTPUT_FILE) $(OBJECT_FILES) "
                    "$(LINK_FLAGS) /link "
                    "{/libpath:\"$(LIBRARY_FOLDERS)\"} "
                    "{$(LIBRARY_FILES).lib}"), // execution_link_format
        ccc::Format("lib /OUT:$(OUTPUT_FILE) $(OBJECT_FILES) "
                    "$(LINK_FLAGS)"), // static_library_link_format
        ccc::Format("link /DLL /OUT:$(OUTPUT_FILE) {$(OBJECT_FILES)} "
                    "$(LINK_FLAGS) "
                    "{/libpath:\"$(LIBRARY_FOLDERS)\"} "
                    "{$(LIBRARY_FILES).lib} ") // shared_library_link_format
    );

    // Add the '/EHsc' compile flag when compiling C++ code
    if (language == ccc::language_type::cpp) {
        tc.execution_compile_format.format += " /EHsc";
        tc.static_library_compile_format.format += " /EHsc";
        tc.shared_library_compile_format.format += " /EHsc";
    } else if (language == ccc::language_type::c) {

    } else {
        std::cerr << "Error: Unsupported language type." << std::endl;
        exit(1);
    }

    return tc;
}

} // namespace built_in_toolchain
} // namespace ccc