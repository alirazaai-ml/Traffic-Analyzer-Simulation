#include "TrafficSystem.h"
#include <iostream>
#include <random>
#include <chrono>

TrafficSystem::TrafficSystem()
    : isRunning(false), simulateTrafficUpdates(false) {
    std::cout << "Traffic System Initializing..." << std::endl;
}

TrafficSystem::~TrafficSystem() {
    stop();
    std::cout << "Traffic System Shutdown." << std::endl;
}

void TrafficSystem::initialize() {
    createSampleCity();

    trafficManager = std::make_unique<TrafficUpdates>(cityMap);

    uiManager = std::make_unique<UIManager>(cityMap);

    isRunning = true;

    std::cout << "Traffic System Ready!" << std::endl;
    printStatistics();
}

void TrafficSystem::run() {
    if (!isRunning) {
        std::cout << "System not initialized. Call initialize() first." << std::endl;
        return;
    }

    std::cout << "\n=== Starting Traffic System ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  GUI Controls apply" << std::endl;
    std::cout << "  Press 'Q' in console to quit" << std::endl;
    std::cout << "==============================\n" << std::endl;

    if (uiManager) {
        uiManager->run();
    }

}

void TrafficSystem::stop() {
    isRunning = false;
    if (trafficManager) {
        trafficManager.reset();
    }
    if (uiManager) {
    }
}

void TrafficSystem::createSampleCity() {
    std::cout << "Creating sample city map..." << std::endl;

    int nodeId = 1;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cityMap.addNode(nodeId++, 100 + i * 80, 100 + j * 80,
                "Downtown_" + std::to_string(i) + "_" + std::to_string(j));
        }
    }

    int edgeId = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int currentNode = i * 4 + j + 1;

            if (j < 3) {
                cityMap.addEdge(edgeId++, currentNode, currentNode + 1, 0.3f, 40, "Local St");
            }

            if (i < 3) {
                cityMap.addEdge(edgeId++, currentNode, currentNode + 4, 0.3f, 40, "Local Ave");
            }
        }
    }

    cityMap.addNode(nodeId++, 50, 50, "Highway_North");
    cityMap.addNode(nodeId++, 50, 350, "Highway_South");
    cityMap.addEdge(edgeId++, 17, 18, 3.0f, 100, "North-South Highway");

    cityMap.addNode(nodeId++, 50, 200, "Highway_West");
    cityMap.addNode(nodeId++, 350, 200, "Highway_East");
    cityMap.addEdge(edgeId++, 19, 20, 3.0f, 100, "East-West Highway");

    cityMap.addEdge(edgeId++, 1, 17, 0.5f, 60, "Downtown Access");
    cityMap.addEdge(edgeId++, 13, 18, 0.5f, 60, "Downtown Access");
    cityMap.addEdge(edgeId++, 4, 19, 0.5f, 60, "Downtown Access");
    cityMap.addEdge(edgeId++, 16, 20, 0.5f, 60, "Downtown Access");

    cityMap.addNode(nodeId++, 400, 100, "North_Suburb");
    cityMap.addNode(nodeId++, 400, 300, "South_Suburb");
    cityMap.addEdge(edgeId++, 20, 21, 2.0f, 80, "Suburb Connector");
    cityMap.addEdge(edgeId++, 20, 22, 2.0f, 80, "Suburb Connector");

    std::cout << "Created city with " << cityMap.getNodeCount()
        << " nodes and " << cityMap.getEdgeCount()
        << " roads." << std::endl;
}

void TrafficSystem::createGridCity(int width, int height, float spacing) {
    std::cout << "Creating grid city..." << std::endl;

    int nodeId = 1;
    int edgeId = 1;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            cityMap.addNode(nodeId++, 100 + j * spacing, 100 + i * spacing,
                "Node_" + std::to_string(i) + "_" + std::to_string(j));
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width - 1; j++) {
            int from = i * width + j + 1;
            int to = from + 1;
            cityMap.addEdge(edgeId++, from, to, spacing / 100.0f, 50, "Street");
        }
    }

    for (int i = 0; i < height - 1; i++) {
        for (int j = 0; j < width; j++) {
            int from = i * width + j + 1;
            int to = from + width;
            cityMap.addEdge(edgeId++, from, to, spacing / 100.0f, 50, "Avenue");
        }
    }

    std::cout << "Grid city created with " << (width * height)
        << " intersections." << std::endl;
}

void TrafficSystem::saveCityMap(const std::string& filename) {
    cityMap.saveToFile(filename);
    std::cout << "City map saved to " << filename << std::endl;
}

void TrafficSystem::loadCityMap(const std::string& filename) {
    cityMap.loadFromFile(filename);
    std::cout << "City map loaded from " << filename << std::endl;

    if (trafficManager) {
        trafficManager.reset();
        trafficManager = std::make_unique<TrafficUpdates>(cityMap);
    }
}

void TrafficSystem::addRandomVehicle() {
    if (cityMap.getNodeCount() < 2) {
        std::cout << "Not enough nodes to add a vehicle." << std::endl;
        return;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::vector<int> nodeIds;
    for (const auto& [id, node] : cityMap.getAllNodes()) {
        nodeIds.push_back(id);
    }

    if (nodeIds.size() < 2) return;

    std::uniform_int_distribution<> dist(0, nodeIds.size() - 1);

    int startNode = nodeIds[dist(gen)];
    int endNode = nodeIds[dist(gen)];

    while (endNode == startNode && nodeIds.size() > 1) {
        endNode = nodeIds[dist(gen)];
    }

    addVehicle(startNode, endNode);
}

void TrafficSystem::addVehicle(int startNode, int endNode) {
    if (!uiManager) {
        std::cout << "UI Manager not initialized." << std::endl;
        return;
    }

    uiManager->addVehicle(startNode, endNode);
    std::cout << "Added vehicle from node " << startNode
        << " to node " << endNode << std::endl;
}

void TrafficSystem::clearAllVehicles() {
    std::cout << "Clearing all vehicles." << std::endl;
}

void TrafficSystem::toggleTrafficSimulation() {
    simulateTrafficUpdates = !simulateTrafficUpdates;

    if (uiManager) {
        uiManager->toggleTrafficSimulation();
    }

    std::cout << "Traffic simulation: "
        << (simulateTrafficUpdates ? "ENABLED" : "DISABLED")
        << std::endl;
}

void TrafficSystem::simulateAccident(int edgeId) {
    if (trafficManager) {
        trafficManager->simulateAccident(edgeId);
    }
}

void TrafficSystem::clearAccident(int edgeId) {
    if (trafficManager) {
        trafficManager->clearAccident(edgeId);
    }
}

void TrafficSystem::simulatePeakHour() {
    if (trafficManager) {
        trafficManager->simulatePeakHour();
    }
    std::cout << "Peak hour traffic simulated." << std::endl;
}

void TrafficSystem::simulateNormalTraffic() {
    std::cout << "Normal traffic conditions restored." << std::endl;
    for (const auto& [edgeId, edge] : cityMap.getAllEdges()) {
        cityMap.updateEdgeTraffic(edgeId, edge.speedLimit * 0.9f);
    }
}

std::vector<int> TrafficSystem::calculateRoute(int startNode, int endNode) {

    if (!cityMap.hasNode(startNode) || !cityMap.hasNode(endNode)) {
        std::cout << "Invalid nodes selected." << std::endl;
        return {};
    }

    return { startNode, endNode };
}

void TrafficSystem::calculateAndDisplayRoute(int startNode, int endNode) {
    auto route = calculateRoute(startNode, endNode);

    if (route.empty()) {
        std::cout << "No route found between nodes "
            << startNode << " and " << endNode << std::endl;
        return;
    }

    std::cout << "Route from " << startNode << " to " << endNode << ": ";
    for (size_t i = 0; i < route.size(); i++) {
        std::cout << route[i];
        if (i < route.size() - 1) std::cout << " -> ";
    }
    std::cout << std::endl;

    if (uiManager) {
        uiManager->calculateRoute();
    }
}

void TrafficSystem::printStatistics() const {
    std::cout << "\n=== System Statistics ===" << std::endl;
    std::cout << "Nodes (Intersections): " << cityMap.getNodeCount() << std::endl;
    std::cout << "Edges (Roads): " << cityMap.getEdgeCount() << std::endl;

    int freeFlow = 0, slow = 0, congested = 0, blocked = 0;
    for (const auto& [id, edge] : cityMap.getAllEdges()) {
        switch (edge.trafficLevel) {
        case TrafficLevel::FREE_FLOW: freeFlow++; break;
        case TrafficLevel::SLOW: slow++; break;
        case TrafficLevel::CONGESTED: congested++; break;
        case TrafficLevel::BLOCKED: blocked++; break;
        }
    }

    std::cout << "\nTraffic Conditions:" << std::endl;
    std::cout << "  Free Flow: " << freeFlow << " roads" << std::endl;
    std::cout << "  Slow: " << slow << " roads" << std::endl;
    std::cout << "  Congested: " << congested << " roads" << std::endl;
    std::cout << "  Blocked: " << blocked << " roads" << std::endl;
    std::cout << "=========================\n" << std::endl;
}

int TrafficSystem::getVehicleCount() const {
    return 0;
}

int TrafficSystem::getCongestedRoadsCount() const {
    int count = 0;
    for (const auto& [id, edge] : cityMap.getAllEdges()) {
        if (edge.trafficLevel == TrafficLevel::CONGESTED ||
            edge.trafficLevel == TrafficLevel::BLOCKED) {
            count++;
        }
    }
    return count;
}

void TrafficSystem::initializeDefaultMap() {
    createSampleCity();
}

void TrafficSystem::updateTraffic() {
    if (trafficManager && simulateTrafficUpdates) {
        trafficManager->updateAllTraffic();
    }
}