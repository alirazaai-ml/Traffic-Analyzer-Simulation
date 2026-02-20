#pragma once
#include "Graph.h"
#include <vector>
#include <string>
#include <iostream>  // Added for std::cout

class MapGenerator {
public:
    // Generate different types of city layouts
    static void generateSimpleGrid(Graph& graph, int gridSize = 8);
    static void generateComplexCity(Graph& graph);
    static void generateHighwayNetwork(Graph& graph);
    static void generateRandomCity(Graph& graph, int nodeCount = 30);

    // Advanced generators
    static void generateNextCity(Graph& graph);

    // Add special features
    static void addTrafficHotspot(Graph& graph, int centerNode, float congestionLevel = 0.8f);
    static void createRoundabout(Graph& graph, float centerX, float centerY, float radius);
    static void addBridge(Graph& graph, int node1, int node2, bool isOverpass = true);

    // Helper functions
    static int getNextNodeId(const Graph& graph);
    static int getNextEdgeId(const Graph& graph);

    static void generateCoastalStyleCity(Graph& graph);

    enum class CityType {
        SIMPLE_GRID = 0,
        COMPLEX_CITY,
        RANDOM_CITY,
        METROPOLIS,
        PLANNED_CITY,
        COASTAL_CITY,
        CITY_TYPE_COUNT
    };

    // Generates and returns a new city map as a Graph object
    static Graph generateCity();

private:
    static void connectGrid(Graph& graph, int rows, int cols,
        float startX, float startY, float spacing);

    static void generateGridCity(Graph& graph);
    static void generateRadialCity(Graph& graph);
    static void generateOrganicCity(Graph& graph);

    // Keep track of current city type
    static CityType currentCityType;
};