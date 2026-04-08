#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

class Logger {
public:
    static inline bool enabled = true;

    static void log(const std::string& message) {
        if (enabled) {
            std::cout << "[" << getTimeString() << "] [LOG] " << message << std::endl;
        }
    }

    static void error(const std::string& message) {
        std::cerr << "[" << getTimeString() << "] [ERROR] " << message << std::endl;
    }

private:
    static std::string getTimeString() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
        return ss.str();
    }
};
