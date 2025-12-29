#pragma once
#include "Graph.h"
#include "MapRenderer.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "CarSimulation.h"  

class UIManager {
private:
    sf::RenderWindow window;
    const Graph& graph;
    MapRenderer mapRenderer;

    CarSimulation carSim;
    bool showCars;

    // UI State
    int selectedStartNode;
    int selectedEndNode;
    std::vector<int> currentRoute;

    // View controls
    float zoomLevel;
    sf::Vector2f viewOffset;
    bool isDragging;
    sf::Vector2i lastMousePos;

    // UI Elements
    sf::Font font;
    sf::Text statusText;
    sf::RectangleShape infoPanel;
    sf::Text legendText;

    // Traffic updates
    bool simulateTraffic;
    float trafficUpdateTimer;

public:
    UIManager(const Graph& g);
    void run();

    void toggleCarDisplay() { showCars = !showCars; }
    void addCarToSimulation(int start, int end);

    // Public methods
    void addVehicle(int startNode, int endNode);
    void calculateRoute();
    void toggleTrafficSimulation();

private:
    void handleEvents();
    void update(float deltaTime);
    void render();
    void drawUI();
    void handleMouseClick(int x, int y);
    void handleKeyPress(sf::Keyboard::Key key);

    sf::Vector2f screenToWorld(int x, int y) const;
    int getNodeAtPosition(int x, int y) const;
};