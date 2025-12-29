#include "GUI.h"
#include "Graph.h"
#include "MapGenerator.h"  
#include <iostream>

int main() {
    try {
        Graph cityMap;

        std::cout << " Traffic Analysis System " << std::endl;
        std::cout << "1. Generating complex city layout..." << std::endl;

        MapGenerator::generateComplexCity(cityMap);

        std::cout << "2. City generated: " << cityMap.getNodeCount()
            << " nodes, " << cityMap.getEdgeCount() << " roads." << std::endl;
        std::cout << "3. Starting GUI..." << std::endl;

        GUI gui(cityMap);
        gui.run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}