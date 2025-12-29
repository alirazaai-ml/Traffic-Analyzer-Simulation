#pragma once
#include "Graph.h"
#include <deque>
#include <random>
#include <chrono>

class TrafficUpdates {
private:
    struct TrafficData {
        std::deque<float> speedReadings;
        const size_t windowSize = 10;

        void addReading(float speed) {
            speedReadings.push_back(speed);
            if (speedReadings.size() > windowSize) {
                speedReadings.pop_front();
            }
        }

        float getAverageSpeed() const {
            if (speedReadings.empty()) return 0;
            float sum = 0;
            for (float speed : speedReadings) {
                sum += speed;
            }
            return sum / speedReadings.size();
        }
    };

    Graph& graph;
    std::unordered_map<int, TrafficData> trafficSensors;
    std::mt19937 randomGenerator;

public:
    TrafficUpdates(Graph& g);
    void updateAllTraffic();
    void addRandomTrafficEvent();
    void simulatePeakHour();
    void simulateAccident(int edgeId);
    void clearAccident(int edgeId);

private:
    void updateEdgeWithSensor(int edgeId);
};
