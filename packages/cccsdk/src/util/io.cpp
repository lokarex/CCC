#include "util/io.h"
#include <cstdio>
#include <mutex>
#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
#elif __linux__
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

static std::mutex out_mutex;

void ccc::io::print(const std::string& message) {
    std::lock_guard<std::mutex> lock(out_mutex);
    printf("%s", message.c_str());
}

void ccc::io::print(const char* message) {
    std::lock_guard<std::mutex> lock(out_mutex);
    printf("%s", message);
}

void ccc::io::println(const std::string& message) {
    std::lock_guard<std::mutex> lock(out_mutex);
    printf("%s\n", message.c_str());
}

void ccc::io::println(const char* message) {
    std::lock_guard<std::mutex> lock(out_mutex);
    printf("%s\n", message);
}

static std::mutex exec_out_mutex;

bool ccc::io::exec_command(const std::string& cmd, bool is_cmd_print,
                           bool is_res_print) {
    // Create the pipe
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(exec_out_mutex);

        // Print the command
        if (is_cmd_print) {
            ccc::io::println(cmd);
        }

        // Print the result
        if (is_res_print) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                ccc::io::print(buffer);
            }
        }
        // Discard the result, and prevent pipeline blockage.
        else {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
                ;
        }
    }

    int status = pclose(pipe);
#ifdef _WIN32
    return (status == 0);
#else
    if (WIFEXITED(status)) {
        return (WEXITSTATUS(status) == 0);
    }
    return false;
#endif
}

int ccc::io::get_terminal_width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

void ccc::io::note(const std::string& message) {
    ccc::io::println("ccc: " + Cyan + "note: " + Reset + message);
}

void ccc::io::warn(const std::string& message) {
    ccc::io::println("ccc: " + Orange + "warn: " + Reset + message);
}

void ccc::io::error(const std::string& message) {
    ccc::io::println("ccc: " + Red + "error: " + Reset + message);
}