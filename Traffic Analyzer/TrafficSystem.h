#pragma once
#include "Graph.h"
#include "TrafficUpdates.h"
#include "UIManager.h"
#include <memory>

class TrafficSystem {
private:
    Graph cityMap;
    std::unique_ptr<TrafficUpdates> trafficManager;
    std::unique_ptr<UIManager> uiManager;

    // Simulation state
    bool isRunning;
    bool simulateTrafficUpdates;

public:
    TrafficSystem();
    ~TrafficSystem();

    void initialize();
    void run();
    void stop();

    // Map management
    void createSampleCity();
    void createGridCity(int width, int height, float spacing);
    void saveCityMap(const std::string& filename);
    void loadCityMap(const std::string& filename);

    // Vehicle management
    void addRandomVehicle();
    void addVehicle(int startNode, int endNode);
    void clearAllVehicles();

    // Traffic management
    void toggleTrafficSimulation();
    void simulateAccident(int edgeId);
    void clearAccident(int edgeId);
    void simulatePeakHour();
    void simulateNormalTraffic();

    // Route calculation
    std::vector<int> calculateRoute(int startNode, int endNode);
    void calculateAndDisplayRoute(int startNode, int endNode);

    // Statistics
    void printStatistics() const;
    int getVehicleCount() const;
    int getCongestedRoadsCount() const;

private:
    void initializeDefaultMap();
    void updateTraffic();
};