#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <vector>

struct PerformanceMetrics {
    float avgFrameTime;
    size_t peakMemoryUsage;
    int activeVehicles;
    int routeCalculations;
    // ... add more metrics
};

class DataLogger {
private:
    std::ofstream logFile;
    std::chrono::time_point<std::chrono::system_clock> sessionStart;
    std::vector<float> frameTimes;
    size_t maxMemoryUsage;

public:
    DataLogger(const std::string& filename);
    ~DataLogger();

    void startSession();
    void logTrafficState(const std::map<std::string, float>& metrics);
    void logRouteCalculation(int start, int end, float distance, float time);
    void logFrameTime(float ms);
    void generateReport(const std::string& filename);
    PerformanceMetrics getPerformanceMetrics() const;
};