#ifndef __FILE_H__
#define __FILE_H__

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

inline bool compareFileModificationTime(const std::string& first,
                                        const std::string& second) {
    auto firstTime = fs::last_write_time(first);
    auto secondTime = fs::last_write_time(second);
    return firstTime < secondTime;
}

#endif