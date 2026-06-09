#ifndef __CCC_FORMAT_H__
#define __CCC_FORMAT_H__

#include <string>
#include <unordered_map>
#include <vector>

namespace ccc {
/* The operating systems currently supported by ccc. */
enum system_type { windows_os, linux_os /*, macos */ };

inline ccc::system_type current_os =
#ifdef _WIN32
    system_type::windows_os;
#else
    system_type::linux_os;
#endif

/* The language currently supported by ccc. */
enum language_type { c, cpp };

class Format {
  public:
    std::string format;

    Format() = default;

    Format(const std::string& format) { this->format = format; }

    Format(const Format& other) { format = other.format; };

    Format& operator=(const Format&) = default;

    bool operator==(const Format& other) const {
        return format == other.format;
    }

    std::string
    replace(std::unordered_map<std::string, std::vector<std::string>>& mp);
};

class toolchain {
  public:
    /* The target operating system of the compiled product. */
    ccc::system_type target_os;
    ccc::language_type language;

    std::string name;

    /* The compiler */
    std::string compiler;
    /* The linker */
    std::string linker;

    ccc::Format compile_format;
    ccc::Format execution_compile_format;
    ccc::Format static_library_compile_format;
    ccc::Format shared_library_compile_format;

    ccc::Format link_format;
    ccc::Format execution_link_format;
    ccc::Format static_library_link_format;
    ccc::Format shared_library_link_format;

    toolchain() = default;

    toolchain(const ccc::system_type& target_os,
              const ccc::language_type& language, const std::string& name,

              const std::string& compiler, const std::string& linker,

              const ccc::Format& compile_format,
              const ccc::Format& execution_compile_format,
              const ccc::Format& static_library_compile_format,
              const ccc::Format& shared_library_compile_format,

              const ccc::Format& link_format,
              const ccc::Format& execution_link_format,
              const ccc::Format& static_library_link_format,
              const ccc::Format& shared_library_link_format)
        : target_os(target_os), language(language), name(name),

          compiler(compiler), linker(linker),

          compile_format(compile_format),
          execution_compile_format(execution_compile_format),
          static_library_compile_format(static_library_compile_format),
          shared_library_compile_format(shared_library_compile_format),

          link_format(link_format),
          execution_link_format(execution_link_format),
          static_library_link_format(static_library_link_format),
          shared_library_link_format(shared_library_link_format) {}

    toolchain(const toolchain& other) = default;

    toolchain& operator=(const toolchain& other) = default;

    bool operator==(const toolchain& other) const {
        return this->target_os == other.target_os &&
               this->language == other.language && this->name == other.name;
    }

    bool operator!=(const toolchain& other) const { return !(*this == other); }

    bool is_empty() const {
        return name.empty() && compiler.empty() && linker.empty() &&

               compile_format.format.empty() &&
               execution_compile_format.format.empty() &&
               static_library_compile_format.format.empty() &&
               shared_library_compile_format.format.empty() &&

               link_format.format.empty() &&
               execution_link_format.format.empty() &&
               static_library_link_format.format.empty() &&
               shared_library_link_format.format.empty();
    }
};

namespace built_in_toolchain {

/**
 * @brief Get the gnu toolchain object.
 *
 * @param target_os The target os.
 * @param language The language.
 * @return ccc::toolchain TThe toolchain object.
 */
ccc::toolchain
gnu_toolchain(ccc::system_type target_os = current_os,
              ccc::language_type language = ccc::language_type::cpp);

inline ccc::toolchain gnu_toolchain(ccc::language_type language) {
    return built_in_toolchain::gnu_toolchain(current_os, language);
}

ccc::toolchain
clang_toolchain(ccc::system_type target_os = current_os,
                ccc::language_type language = ccc::language_type::cpp);
inline ccc::toolchain clang_toolchain(ccc::language_type language) {
    return built_in_toolchain::clang_toolchain(current_os, language);
}

ccc::toolchain
msvc_toolchain(ccc::system_type target_os = current_os,
               ccc::language_type language = ccc::language_type::cpp);
inline ccc::toolchain msvc_toolchain(ccc::language_type language) {
    return built_in_toolchain::msvc_toolchain(current_os, language);
}

} // namespace built_in_toolchain

} // namespace ccc

#endif