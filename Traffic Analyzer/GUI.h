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

    // Systems
    AccidentSystem* accidentSystem;
    PredictionSystem* predictionSystem;
    CarSimulation* carSim;
    
    // Prediction settings
    bool showPredictions;
    sf::Color predictedCongestionColor;

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
    Button generateCityBtn;
    Button spawnManyCarsBtn;
    Button rushHourBtn;
    Button clearTrafficBtn;
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
    bool showCars;
    float simulationSpeed;
    int totalCarsSpawned;

    sf::Clock guiClock;

public:
    GUI(Graph& map);
    ~GUI();

    void run();
    void addCar(int startNode, int endNode);
    void setCurrentPath(const std::vector<int>& path);
    void updateStatistics(int nodeCount, int edgeCount, int carCount, float avgSpeed);
    PredictionSystem* getPredictionSystem() { return predictionSystem; }

private:
    // Initialization methods
    void initializeFont();
    void initializeWarningTexture();
    void initializeSystems();
    void initializeUI();
    void initializeControlPanel();
    void initializeInputFields();
    void initializeButtons();
    void initializeStatistics();

    // Event handling
    void handleEvents();
    void handleMouseClick(int x, int y);
    void handleControlPanelClick(const sf::Vector2f& screenPos);
    void handleMapClick(int x, int y);
    void handleNodeSelection(int nodeId);
    void handleTextInput(sf::Uint32 unicode);
    void handleBackspace();
    void handleDigitInput(char digit);
    void handleEnterKey();
    void handleButtonClick(const sf::Vector2f& mousePos);

    // Update methods
    void update();
    void updateSimulation(float deltaTime);
    void updateAccidentVisuals();

    // Rendering methods
    void render();
    void drawControlPanel();
    void drawMap();
    void drawNode(const Node& node, bool isSelected);
    void drawEdge(const Edge& edge);
    void drawPath();
    void drawCars();
    void drawPredictions();

    // Helper methods
    void createButton(Button& btn, float x, float y, float w, float h, const std::string& text);
    void drawButton(const Button& btn);
    int findNodeAtPosition(const sf::Vector2f& worldPos) const;
};