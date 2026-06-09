#include "ccc/config.h"

void ccc::config_manager::set_config(const ccc::config& cfg) {
    this->config = cfg;
}

void ccc::config_manager::set_compile_flags(
    const std::initializer_list<std::string>& compile_flags) {

    this->config.compile_flags = compile_flags;
}

void ccc::config_manager::add_compile_flags(
    const std::initializer_list<std::string>& compile_flags) {
    this->config.compile_flags.insert(this->config.compile_flags.end(),
                                      compile_flags.begin(),
                                      compile_flags.end());
}

void ccc::config_manager::set_link_flags(
    const std::initializer_list<std::string>& link_flags) {

    this->config.link_flags = link_flags;
}

void ccc::config_manager::add_link_flags(
    const std::initializer_list<std::string>& link_flags) {
    this->config.link_flags.insert(this->config.link_flags.end(),
                                   link_flags.begin(), link_flags.end());
}

void ccc::config_manager::set_thread_num(uint32_t num) {
    this->config.thread_num = num;
}

void ccc::config_manager::set_header_folder_paths(
    const std::initializer_list<std::string>& header_folder_paths) {
    this->config.header_folder_paths = header_folder_paths;
}

void ccc::config_manager::add_header_folder_paths(
    const std::initializer_list<std::string>& header_folder_paths) {
    this->config.header_folder_paths.insert(
        this->config.header_folder_paths.end(), header_folder_paths.begin(),
        header_folder_paths.end());
}

void ccc::config_manager::set_marcos(
    const std::initializer_list<std::string>& macros) {
    this->config.macros = macros;
}

void ccc::config_manager::add_marcos(
    const std::initializer_list<std::string>& macros) {
    this->config.macros.insert(this->config.macros.end(), macros.begin(),
                               macros.end());
}