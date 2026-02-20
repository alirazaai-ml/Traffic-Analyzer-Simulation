#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

// Error severity levels
enum class ErrorLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// Logger class for consistent error reporting
class Logger {
private:
    static std::ofstream logFile;
    static bool fileLoggingEnabled;

public:
    static void init(const std::string& filename = "traffic_analyzer.log") {
        logFile.open(filename, std::ios::app);
        if (logFile.is_open()) {
            fileLoggingEnabled = true;
            log(ErrorLevel::INFO, "Logger initialized");
        }
    }

    static void shutdown() {
        if (fileLoggingEnabled) {
            log(ErrorLevel::INFO, "Logger shutting down");
            logFile.close();
            fileLoggingEnabled = false;
        }
    }

    static void log(ErrorLevel level, const std::string& message) {
        std::string timestamp = getTimestamp();
        std::string levelStr = getLevelString(level);
        std::string fullMessage = timestamp + " [" + levelStr + "] " + message;

        // Console output
        if (level == ErrorLevel::ERROR || level == ErrorLevel::CRITICAL) {
            std::cerr << fullMessage << std::endl;
        } else {
            std::cout << fullMessage << std::endl;
        }

        // File output
        if (fileLoggingEnabled && logFile.is_open()) {
            logFile << fullMessage << std::endl;
            logFile.flush();
        }
    }

    static void info(const std::string& message) {
        log(ErrorLevel::INFO, message);
    }

    static void warning(const std::string& message) {
        log(ErrorLevel::WARNING, message);
    }

    static void error(const std::string& message) {
        log(ErrorLevel::ERROR, message);
    }

    static void critical(const std::string& message) {
        log(ErrorLevel::CRITICAL, message);
    }

private:
    static std::string getTimestamp() {
        time_t now = time(nullptr);
        struct tm timeinfo;
        char buf[80];
        
#ifdef _WIN32
        localtime_s(&timeinfo, &now);  // Windows safe version
#else
        localtime_r(&now, &timeinfo);   // Unix safe version
#endif
        
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return std::string(buf);
    }

    static std::string getLevelString(ErrorLevel level) {
        switch (level) {
            case ErrorLevel::INFO:     return "INFO";
            case ErrorLevel::WARNING:  return "WARN";
            case ErrorLevel::ERROR:    return "ERROR";
            case ErrorLevel::CRITICAL: return "CRITICAL";
            default:                   return "UNKNOWN";
        }
    }
};

// Initialize static members
std::ofstream Logger::logFile;
bool Logger::fileLoggingEnabled = false;

// Helper macros for convenient logging
#define LOG_INFO(msg) Logger::info(msg)
#define LOG_WARNING(msg) Logger::warning(msg)
#define LOG_ERROR(msg) Logger::error(msg)
#define LOG_CRITICAL(msg) Logger::critical(msg)
