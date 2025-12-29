#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Graph.h"
#include "AccidentSystem.h"
#include "PredictionSystem.h"

// Forward declarations
class Graph;
class CarSimulation;

class GUI {
private:
    sf::RenderWindow window;
    Graph& cityMap;

    // Accident visuals
    sf::Texture warningTexture;
    sf::Sprite warningSprite;
    std::vector<sf::Sprite> accidentIcons;

    // UI Elements
    sf::Font font;

    // Control Panel
    sf::RectangleShape controlPanel;
    sf::Text panelTitle;

    // Input fields
    sf::Text sourceLabel, destLabel;
    sf::RectangleShape sourceBox, destBox;
    std::string sourceText, destText;
    bool sourceActive, destActive;

    AccidentSystem* accidentSystem;


    PredictionSystem* predictionSystem;  // ADD THIS
    bool showPredictions;                // Toggle prediction display

    // Prediction colors
    sf::Color predictedCongestionColor;  // Purple for predicted congestion

    // Prediction overlay
    void drawPredictions();


    // Buttons
    struct Button {
        sf::RectangleShape shape;
        sf::Text label;
    };

    Button findRouteBtn;
    Button addCarBtn;
    Button clearCarsBtn;
    Button trafficBtn;
    Button peakHourBtn;
    Button accidentBtn;


    // Add to Button struct section in GUI.h
    Button generateCityBtn;
    Button spawnManyCarsBtn;
    Button rushHourBtn;
    Button clearTrafficBtn;
    Button toggleHeatmapBtn;

    Button clearAccidentsBtn;
    Button togglePredictionsBtn;


    // Statistics
    sf::Text statsText;

    // Map View
    float zoomLevel;
    sf::Vector2f viewOffset;
    bool isDragging;
    sf::Vector2i lastMousePos;

    // Selection
    int selectedStartNode;
    int selectedEndNode;
    std::vector<int> currentPath;

    // Colors
    sf::Color freeFlowColor;
    sf::Color slowColor;
    sf::Color congestedColor;
    sf::Color blockedColor;

    // Car simulation
    CarSimulation* carSim;
    bool showCars;
    float simulationSpeed;
    int totalCarsSpawned;

    sf::Clock guiClock;

    // Method to update accident icons
    void updateAccidentVisuals();

public:
    GUI(Graph& map);
    ~GUI();

    void run();
    void addCar(int startNode, int endNode);

    // Public methods
    void setCurrentPath(const std::vector<int>& path);
    void updateStatistics(int nodeCount, int edgeCount, int carCount, float avgSpeed);
    PredictionSystem* getPredictionSystem() { return predictionSystem; }

private:
    void handleEvents();
    void update();
    void render();
    void updateSimulation(float deltaTime);

    // Drawing methods
    void drawControlPanel();
    void drawMap();
    void drawNode(const Node& node, bool isSelected);
    void drawEdge(const Edge& edge);
    void drawPath();
    void drawCars();

    // Input handling
    void handleMouseClick(int x, int y);
    void handleTextInput(sf::Uint32 unicode);
    void handleButtonClick(const sf::Vector2f& mousePos);

    // Helper methods
    void createButton(Button& btn, float x, float y, float w, float h, const std::string& text);
    void drawButton(const Button& btn);
};