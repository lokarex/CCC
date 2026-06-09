#ifndef __FILE_H__
#define __FILE_H__

#include <filesystem>
#include <numeric>
#include <string>
#include <vector>

namespace fs = std::filesystem;

inline std::string joinWithSpace(const std::vector<std::string>& vec) {
    return std::accumulate(vec.begin(), vec.end(), std::string(),
                           [](const std::string& a, const std::string& b) {
                               return a.empty() ? b : a + " " + b;
                           });
}

inline std::string changeFileExtensionToO(const std::string& filePath) {
    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos && dotPos > filePath.find_last_of("/\\")) {
        return filePath.substr(0, dotPos) + ".o";
    } else {
        return filePath + ".o";
    }
}

inline bool directoryExists(const std::string& path) {
    return fs::exists(path) && fs::is_directory(path);
}

inline std::string extractPath(const std::string& fullPath) {
    size_t found = fullPath.find_last_of("/\\");
    if (found != std::string::npos) {
        return fullPath.substr(0, found);
    }
    return "";
}

inline bool compareFileModificationTime(const std::string& first,
                                        const std::string& second) {
    auto firstTime = fs::last_write_time(first);
    auto secondTime = fs::last_write_time(second);
    return firstTime < secondTime;
}

inline std::string changeFileExtension(const std::string& filePath,
                                       const std::string& newExtension) {
    fs::path path(filePath);
    path.replace_extension(newExtension);
    return path.string();
}

#endif