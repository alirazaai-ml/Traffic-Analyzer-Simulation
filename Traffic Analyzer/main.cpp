#include "GUI.h"
#include "Graph.h"
#include "MapGenerator.h"
#include "Logger.h"
#include <iostream>

int main() {
    try {
        // Initialize logging system
        Logger::init("traffic_analyzer.log");
        LOG_INFO("=== Traffic Analysis System Starting ===");

        Graph cityMap;

        LOG_INFO("Generating complex city layout...");
        MapGenerator::generateComplexCity(cityMap);

        LOG_INFO("City generated: " + std::to_string(cityMap.getNodeCount()) +
            " nodes, " + std::to_string(cityMap.getEdgeCount()) + " roads");

        LOG_INFO("Starting GUI...");
        GUI gui(cityMap);
        gui.run();

        LOG_INFO("=== Traffic Analysis System Shutting Down ===");
        Logger::shutdown();

        return 0;
    }
    catch (const std::exception& e) {
        LOG_CRITICAL(std::string("Fatal error: ") + e.what());
        Logger::shutdown();
        return 1;
    }
}