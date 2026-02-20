#include "GUI.h"
#include "Graph.h"
#include "CarSimulation.h"
#include "MapGenerator.h"
#include "Config.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <ctime>

// Constructor
GUI::GUI(Graph& map)
    : cityMap(map),
    window(sf::VideoMode(
        static_cast<unsigned int>(UIConfig::WINDOW_WIDTH),
        static_cast<unsigned int>(UIConfig::WINDOW_HEIGHT)
    ), "Traffic Analysis System"),
    zoomLevel(1.0f),
    viewOffset(0.0f, 0.0f),
    isDragging(false),
    selectedStartNode(-1),
    selectedEndNode(-1),
    sourceActive(false),
    destActive(false),
    carSim(nullptr),
    showCars(true),
    simulationSpeed(SimConfig::DEFAULT_SIMULATION_SPEED),
    totalCarsSpawned(0),
    freeFlowColor(ColorConfig::FREE_FLOW_R, ColorConfig::FREE_FLOW_G, ColorConfig::FREE_FLOW_B),
    slowColor(ColorConfig::SLOW_R, ColorConfig::SLOW_G, ColorConfig::SLOW_B),
    congestedColor(ColorConfig::CONGESTED_R, ColorConfig::CONGESTED_G, ColorConfig::CONGESTED_B),
    blockedColor(ColorConfig::BLOCKED_R, ColorConfig::BLOCKED_G, ColorConfig::BLOCKED_B),
    showPredictions(false),
    predictedCongestionColor(ColorConfig::PREDICTED_CONGESTION_R, 
                            ColorConfig::PREDICTED_CONGESTION_G, 
                            ColorConfig::PREDICTED_CONGESTION_B)
{
    std::cout << "Initializing GUI..." << std::endl;

    initializeFont();
    initializeWarningTexture();
    initializeSystems();
    initializeUI();

    std::cout << "GUI initialization complete!" << std::endl;
}

GUI::~GUI() {
    delete carSim;
    delete accidentSystem;
    delete predictionSystem;
}

void GUI::run() {
    sf::Clock clock;
    bool needsForceRedraw = false;  

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        handleEvents();
        update();
        render();

        if (needsForceRedraw) {
            window.clear(sf::Color(25, 25, 35));
            render();
            needsForceRedraw = false;
        }
    }
}

void GUI::updateSimulation(float deltaTime) {
    if (carSim) {
        carSim->update(deltaTime);
    }

    updateStatistics(
        cityMap.getNodeCount(),
        cityMap.getEdgeCount(),
        carSim ? carSim->getVehicleCount() : 0,
        45.5f * simulationSpeed
    );
}

void GUI::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
        case sf::Event::Closed:
            window.close();
            break;

        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left) {
                handleMouseClick(event.mouseButton.x, event.mouseButton.y);
            }
            else if (event.mouseButton.button == sf::Mouse::Right) {
                isDragging = true;
                lastMousePos = sf::Mouse::getPosition(window);
            }
            break;

        case sf::Event::MouseButtonReleased:
            if (event.mouseButton.button == sf::Mouse::Right) {
                isDragging = false;
            }
            break;

        case sf::Event::MouseMoved:
            if (isDragging) {
                sf::Vector2i currentPos = sf::Mouse::getPosition(window);
                sf::Vector2f delta(
                    static_cast<float>(currentPos.x - lastMousePos.x),
                    static_cast<float>(currentPos.y - lastMousePos.y)
                );
                viewOffset += delta * (1.0f / zoomLevel);
                lastMousePos = currentPos;
            }
            break;

        case sf::Event::MouseWheelScrolled:
            if (event.mouseWheelScroll.delta > 0) {
                zoomLevel *= 1.1f;
            }
            else {
                zoomLevel *= 0.9f;
            }
            zoomLevel = std::max(0.1f, std::min(zoomLevel, 5.0f));
            break;

        case sf::Event::TextEntered:
            handleTextInput(event.text.unicode);
            break;

        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            break;

        default:
            break;
        }
    }
}

void GUI::update() {
    updateSimulation(1.0f / 60.0f);


    if (accidentSystem) {
        accidentSystem->update(1.0f / 60.0f);
    }
    if (predictionSystem) {
        predictionSystem->update(1.0f / 60.0f);
    }
    cityMap.updateAccidents(1.0f / 60.0f); 

    updateAccidentVisuals();

    // Update button states
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    auto updateBtn = [&mousePos](Button& btn) {
        bool isHovered = btn.shape.getGlobalBounds().contains(mousePos);
        btn.shape.setFillColor(isHovered ? sf::Color(100, 150, 200) : sf::Color(70, 130, 180));
        };

    updateBtn(findRouteBtn);
    updateBtn(addCarBtn);
    updateBtn(clearCarsBtn);
    updateBtn(trafficBtn);
    updateBtn(peakHourBtn);
    updateBtn(accidentBtn);

    updateBtn(generateCityBtn);      
    updateBtn(spawnManyCarsBtn);     
    updateBtn(rushHourBtn);          
    updateBtn(clearTrafficBtn);      

    updateBtn(clearAccidentsBtn);
    updateBtn(togglePredictionsBtn);

    sourceBox.setOutlineColor(sourceActive ? sf::Color::Cyan : sf::Color(150, 150, 150));
    destBox.setOutlineColor(destActive ? sf::Color::Cyan : sf::Color(150, 150, 150));
}

void GUI::render() {
    window.clear(sf::Color(25, 25, 35));

    sf::View mapView = window.getDefaultView();
    mapView.setSize(sf::Vector2f(900.0f / zoomLevel, 800.0f / zoomLevel));
    mapView.setCenter(sf::Vector2f(450.0f / zoomLevel + viewOffset.x,
        400.0f / zoomLevel + viewOffset.y));
    window.setView(mapView);

    drawMap();

    window.setView(window.getDefaultView());
    drawControlPanel();

    window.display();
}

void GUI::drawControlPanel() {
    window.draw(controlPanel);
    window.draw(panelTitle);

    window.draw(sourceLabel);
    window.draw(sourceBox);
    window.draw(destLabel);
    window.draw(destBox);

    if (!sourceText.empty()) {
        sf::Text sourceDisplay;
        sourceDisplay.setFont(font);
        sourceDisplay.setString(sourceText);
        sourceDisplay.setCharacterSize(16); 
        sourceDisplay.setFillColor(sf::Color::White);
        sourceDisplay.setPosition(930.0f, 88.0f);
        window.draw(sourceDisplay);
    }

    if (!destText.empty()) {
        sf::Text destDisplay;
        destDisplay.setFont(font);
        destDisplay.setString(destText);
        destDisplay.setCharacterSize(16); 
        destDisplay.setFillColor(sf::Color::White);
        destDisplay.setPosition(930.0f, 153.0f);
        window.draw(destDisplay);
    }

    // Draw buttons
    drawButton(findRouteBtn);
    drawButton(addCarBtn);
    drawButton(clearCarsBtn);
    drawButton(trafficBtn);
    drawButton(peakHourBtn);
    drawButton(accidentBtn);
    drawButton(generateCityBtn);      
    drawButton(spawnManyCarsBtn);     
    drawButton(rushHourBtn);          
    drawButton(clearTrafficBtn);      
    drawButton(clearAccidentsBtn);
    drawButton(togglePredictionsBtn);

    // Draw statistics
    window.draw(statsText);

    // Draw Edge Flow section 
    sf::Text edgeFlowTitle;
    edgeFlowTitle.setFont(font);
    edgeFlowTitle.setString("\n\nEdge Flow:");
    edgeFlowTitle.setCharacterSize(14);
    edgeFlowTitle.setFillColor(sf::Color::White);
    edgeFlowTitle.setStyle(sf::Text::Bold);
    edgeFlowTitle.setPosition(920.0f, 520.0f);
    window.draw(edgeFlowTitle);

    sf::Text edgeFlowText;
    edgeFlowText.setFont(font);
    edgeFlowText.setString("\n\nEdge_None\n+Road\nCongested");
    edgeFlowText.setCharacterSize(12);
    edgeFlowText.setFillColor(sf::Color::White);
    edgeFlowText.setLineSpacing(0.9f);
    edgeFlowText.setPosition(920.0f, 545.0f);
    window.draw(edgeFlowText);

    // Draw legend 
    sf::Text legendTitle;
    legendTitle.setFont(font);
    legendTitle.setString("Traffic Legend:");
    legendTitle.setCharacterSize(14); 
    legendTitle.setFillColor(sf::Color::White);
    legendTitle.setStyle(sf::Text::Bold);
    legendTitle.setPosition(920.0f, 620.0f);
    window.draw(legendTitle);

    float yPos = 645.0f;
    std::vector<std::pair<std::string, sf::Color>> legendItems = {
        {"Free Flow", freeFlowColor},
        {"Slow", slowColor},
        {"Congested", congestedColor},
        {"Blocked", blockedColor}
    };

    for (const auto& item : legendItems) {
        sf::RectangleShape colorBox(sf::Vector2f(15.0f, 15.0f)); 
        colorBox.setFillColor(item.second);
        colorBox.setPosition(920.0f, yPos);
        window.draw(colorBox);

        sf::Text itemText;
        itemText.setFont(font);
        itemText.setString(item.first);
        itemText.setCharacterSize(12); 
        itemText.setFillColor(sf::Color::White);
        itemText.setPosition(940.0f, yPos - 2.0f);
        window.draw(itemText);

        yPos += 22.0f; 
    }
}


void GUI::drawMap() {
    auto edges = cityMap.getAllEdges();
    for (const auto& pair : edges) {
        drawEdge(pair.second);
    }

    drawPredictions();
    drawPath();

    for (const auto& icon : accidentIcons) {
        window.draw(icon);
    }

    auto nodes = cityMap.getAllNodes();
    for (const auto& pair : nodes) {
        bool isSelected = (pair.first == selectedStartNode || pair.first == selectedEndNode);
        drawNode(pair.second, isSelected);
    }

    drawCars();
}

void GUI::drawCars() {
    if (carSim && showCars) {
        carSim->draw(window, zoomLevel, viewOffset);
    }
}

void GUI::drawNode(const Node& node, bool isSelected) {
    float screenX = node.x * zoomLevel + viewOffset.x;
    float screenY = node.y * zoomLevel + viewOffset.y;

    // Draw node circle
    sf::CircleShape circle(8.0f * zoomLevel); 
    circle.setPosition(screenX - 8.0f * zoomLevel, screenY - 8.0f * zoomLevel);

    if (isSelected) {
        circle.setFillColor(sf::Color::Cyan);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(1.5f * zoomLevel);
    }
    else {
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(sf::Color(100, 100, 100));
        circle.setOutlineThickness(1.0f * zoomLevel);
    }

    window.draw(circle);

    sf::Text idText;
    idText.setFont(font);
    idText.setString(std::to_string(node.id));
    idText.setCharacterSize(static_cast<unsigned int>(10 * std::min(zoomLevel, 2.0f)));
    idText.setFillColor(sf::Color::Black);
    idText.setStyle(sf::Text::Bold);

    sf::FloatRect textBounds = idText.getLocalBounds();
    idText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    idText.setPosition(screenX, screenY - 1.5f * zoomLevel);

    window.draw(idText);
}

void GUI::drawEdge(const Edge& edge) {
    Node fromNode = cityMap.getNode(edge.fromNodeId);
    Node toNode = cityMap.getNode(edge.toNodeId);

    if (fromNode.id == -1 || toNode.id == -1) return;

    float fromX = fromNode.x * zoomLevel + viewOffset.x;
    float fromY = fromNode.y * zoomLevel + viewOffset.y;
    float toX = toNode.x * zoomLevel + viewOffset.x;
    float toY = toNode.y * zoomLevel + viewOffset.y;

    sf::Color roadColor;
    float roadWidth = 3.0f * zoomLevel; 

    switch (edge.trafficLevel) {
    case TrafficLevel::FREE_FLOW: roadColor = freeFlowColor; break;
    case TrafficLevel::SLOW: roadColor = slowColor; break;
    case TrafficLevel::CONGESTED: roadColor = congestedColor; break;
    case TrafficLevel::BLOCKED: roadColor = blockedColor; break;
    default: roadColor = sf::Color::White;
    }

    if (edge.isBlocked && accidentSystem) {
        roadColor = accidentSystem->getEdgeColorWithAccident(edge.id, roadColor);

        roadWidth = 5.0f * zoomLevel;
    }

    // Draw road
    sf::Vector2f direction(toX - fromX, toY - fromY);
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length < 0.1f) return;

    float angle = std::atan2(direction.y, direction.x) * 180.0f / 3.14159265f;


    sf::RectangleShape road;
    road.setSize(sf::Vector2f(length, roadWidth)); 
    road.setPosition(fromX, fromY);
    road.setRotation(angle);
    road.setFillColor(roadColor);

    window.draw(road);
}

void GUI::drawPath() {
    if (currentPath.size() < 2) return;

    for (size_t i = 0; i < currentPath.size() - 1; i++) {
        Node fromNode = cityMap.getNode(currentPath[i]);
        Node toNode = cityMap.getNode(currentPath[i + 1]);

        if (fromNode.id == -1 || toNode.id == -1) continue;

        float fromX = fromNode.x * zoomLevel + viewOffset.x;
        float fromY = fromNode.y * zoomLevel + viewOffset.y;
        float toX = toNode.x * zoomLevel + viewOffset.x;
        float toY = toNode.y * zoomLevel + viewOffset.y;

        // Draw path segment
        sf::Vector2f direction(toX - fromX, toY - fromY);
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length < 0.1f) continue;

        float angle = std::atan2(direction.y, direction.x) * 180.0f / 3.14159265f;

        sf::RectangleShape pathSegment;
        pathSegment.setSize(sf::Vector2f(length, 6.0f * zoomLevel)); // Thinner
        pathSegment.setPosition(fromX, fromY);
        pathSegment.setRotation(angle);
        pathSegment.setFillColor(sf::Color(0, 150, 255, 180));

        window.draw(pathSegment);
    }
}


void GUI::drawPredictions() {
    if (!predictionSystem || !showPredictions) return;

    auto congestedEdges = predictionSystem->getEdgesLikelyToCongest(5);

    for (int edgeId : congestedEdges) {
        Edge edge = cityMap.getEdge(edgeId);
        if (edge.id == -1) continue;

        Node fromNode = cityMap.getNode(edge.fromNodeId);
        Node toNode = cityMap.getNode(edge.toNodeId);

        if (fromNode.id == -1 || toNode.id == -1) continue;

        float fromX = fromNode.x * zoomLevel + viewOffset.x;
        float fromY = fromNode.y * zoomLevel + viewOffset.y;
        float toX = toNode.x * zoomLevel + viewOffset.x;
        float toY = toNode.y * zoomLevel + viewOffset.y;

        // Draw prediction overlay
        sf::Vector2f direction(toX - fromX, toY - fromY);
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length < 0.1f) continue;

        float angle = std::atan2(direction.y, direction.x) * 180.0f / 3.14159265f;

        sf::RectangleShape predictionOverlay;
        predictionOverlay.setSize(sf::Vector2f(length, 8.0f * zoomLevel));
        predictionOverlay.setPosition(fromX, fromY);
        predictionOverlay.setRotation(angle);
        predictionOverlay.setFillColor(sf::Color(128, 0, 128, 180));

        window.draw(predictionOverlay);
    }
}

void GUI::handleMouseClick(int x, int y) {
    sf::Vector2f screenPos = window.mapPixelToCoords(sf::Vector2i(x, y), window.getDefaultView());

    if (screenPos.x >= UIConfig::CONTROL_PANEL_X) {
        handleControlPanelClick(screenPos);
        return;
    }

    handleMapClick(x, y);
}

void GUI::handleButtonClick(const sf::Vector2f& screenPos) {
    // Find Path button
    if (findRouteBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Find Path clicked - Finding route from " << selectedStartNode
            << " to " << selectedEndNode << std::endl;

        if (selectedStartNode != -1 && selectedEndNode != -1) {
            currentPath = cityMap.findShortestPath(selectedStartNode, selectedEndNode);

            if (!currentPath.empty()) {
                std::cout << "Path found with " << currentPath.size() << " nodes" << std::endl;

                // Calculate and display travel time
                float totalTime = 0.0f;
                for (size_t i = 0; i < currentPath.size() - 1; i++) {
                    int edgeId = cityMap.findEdgeId(currentPath[i], currentPath[i + 1]);
                    if (edgeId != -1) {
                        totalTime += cityMap.getEdge(edgeId).currentTravelTime;
                    }
                }
                std::cout << "Estimated travel time: " << totalTime << " minutes" << std::endl;
            }
            else {
                std::cout << "No path found between nodes!" << std::endl;
            }
        }
        else {
            std::cout << "Please select both source and destination nodes first!" << std::endl;
        }
    }

    // Add Car button
    else if (addCarBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Add Car clicked" << std::endl;

        if (selectedStartNode != -1 && selectedEndNode != -1) {
            std::vector<int> path = cityMap.findShortestPath(selectedStartNode, selectedEndNode);
            if (!path.empty() && carSim) {
                carSim->addCar(selectedStartNode, selectedEndNode, path);
                totalCarsSpawned++;
                std::cout << "Car added! Total cars: " << carSim->getVehicleCount() << std::endl;
            }
        }
        else {
            std::cout << "Please select both source and destination nodes first!" << std::endl;
        }
    }

    // Clear Cars button
    else if (clearCarsBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Clear Cars clicked" << std::endl;

        if (carSim) {
            carSim->clearAllCars();
            std::cout << "All vehicles removed" << std::endl;
        }
    }

    // Traffic Sim button (toggle auto-spawning)
    else if (trafficBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Traffic Sim clicked - Toggling auto-spawn" << std::endl;

        static bool autoSpawnEnabled = false;
        autoSpawnEnabled = !autoSpawnEnabled;

        if (carSim) {
            // You would need to add an auto-spawn toggle in CarSimulation
            std::cout << "Auto-spawn " << (autoSpawnEnabled ? "enabled" : "disabled") << std::endl;

            // Change button color to indicate state
            trafficBtn.shape.setFillColor(autoSpawnEnabled ?
                sf::Color(100, 200, 100) : sf::Color(70, 130, 180));
        }
    }

    // Peak Hour button - spawn 30 cars
    else if (peakHourBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Peak Hour clicked - Spawning 30 cars" << std::endl;

        if (carSim && cityMap.getNodeCount() > 0) {
            auto nodes = cityMap.getAllNodes();
            if (nodes.size() >= 2) {
                std::vector<int> nodeIds;
                for (const auto& pair : nodes) {
                    nodeIds.push_back(pair.first);
                }

                int spawned = 0;
                for (int i = 0; i < 30; i++) {
                    int startIdx = rand() % nodeIds.size();
                    int endIdx = rand() % nodeIds.size();

                    if (startIdx != endIdx) {
                        int start = nodeIds[startIdx];
                        int end = nodeIds[endIdx];

                        std::vector<int> path = cityMap.findShortestPath(start, end);
                        if (!path.empty()) {
                            carSim->addCar(start, end, path);
                            spawned++;
                        }
                    }
                }
                totalCarsSpawned += spawned;
                std::cout << "Spawned " << spawned << " cars. Total: "
                    << carSim->getVehicleCount() << std::endl;
            }
        }
    }

    // Accident button - create random accident
    else if (accidentBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Accident clicked - Creating random accident" << std::endl;

        if (accidentSystem && cityMap.getEdgeCount() > 0) {
            accidentSystem->createRandomAccident();
            std::cout << "Random accident created. Active accidents: "
                << accidentSystem->getActiveAccidentCount() << std::endl;
        }
    }

    // Generate City button
    else if (generateCityBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Generate City clicked" << std::endl;

        cityMap = MapGenerator::generateCity();

        // Reset systems with new map
        delete carSim;
        delete accidentSystem;
        delete predictionSystem;

        initializeSystems();

        // Clear selections
        selectedStartNode = -1;
        selectedEndNode = -1;
        sourceText = "";
        destText = "";
        currentPath.clear();

        std::cout << "New city generated with " << cityMap.getNodeCount()
            << " nodes and " << cityMap.getEdgeCount() << " edges" << std::endl;
    }

    // 20 Cars button
    else if (spawnManyCarsBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "20 Cars clicked - Spawning 20 random cars" << std::endl;

        if (carSim && cityMap.getNodeCount() > 0) {
            auto nodes = cityMap.getAllNodes();
            if (nodes.size() >= 2) {
                std::vector<int> nodeIds;
                for (const auto& pair : nodes) {
                    nodeIds.push_back(pair.first);
                }

                int spawned = 0;
                for (int i = 0; i < 20; i++) {
                    int startIdx = rand() % nodeIds.size();
                    int endIdx = rand() % nodeIds.size();

                    if (startIdx != endIdx) {
                        int start = nodeIds[startIdx];
                        int end = nodeIds[endIdx];

                        std::vector<int> path = cityMap.findShortestPath(start, end);
                        if (!path.empty()) {
                            carSim->addCar(start, end, path);
                            spawned++;
                        }
                    }
                }
                totalCarsSpawned += spawned;
                std::cout << "Spawned " << spawned << " cars" << std::endl;
            }
        }
    }

    // Rush Hour button - heavy traffic
    else if (rushHourBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Rush Hour clicked - Creating heavy traffic" << std::endl;

        if (carSim && cityMap.getNodeCount() > 0) {
            auto nodes = cityMap.getAllNodes();
            if (nodes.size() >= 2) {
                std::vector<int> nodeIds;
                for (const auto& pair : nodes) {
                    nodeIds.push_back(pair.first);
                }

                // Spawn 50 cars
                int spawned = 0;
                for (int i = 0; i < 50; i++) {
                    int startIdx = rand() % nodeIds.size();
                    int endIdx = rand() % nodeIds.size();

                    if (startIdx != endIdx) {
                        int start = nodeIds[startIdx];
                        int end = nodeIds[endIdx];

                        std::vector<int> path = cityMap.findShortestPath(start, end);
                        if (!path.empty()) {
                            carSim->addCar(start, end, path);
                            spawned++;
                        }
                    }
                }
                totalCarsSpawned += spawned;

                // Increase congestion on all edges
                auto edges = cityMap.getAllEdges();
                for (auto& pair : edges) {
                    Edge& edge = pair.second;
                    if (rand() % 100 < 70) { // 70% chance of congestion
                        edge.trafficLevel = TrafficLevel::CONGESTED;
                        edge.currentTravelTime = edge.baseTravelTime * 2.5f;
                    }
                    else if (rand() % 100 < 30) { // 30% chance of slow traffic
                        edge.trafficLevel = TrafficLevel::SLOW;
                        edge.currentTravelTime = edge.baseTravelTime * 1.5f;
                    }
                }

                std::cout << "Rush hour created with " << spawned << " cars and heavy congestion" << std::endl;
            }
        }
    }

    // Clear All button
    else if (clearTrafficBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Clear All clicked - Resetting everything" << std::endl;

        if (carSim) {
            carSim->clearAllCars();
        }

        if (accidentSystem) {
            accidentSystem->clearAllAccidents();
        }

        // Reset all edges to free flow
        auto edges = cityMap.getAllEdges();
        for (auto& pair : edges) {
            Edge& edge = pair.second;
            edge.trafficLevel = TrafficLevel::FREE_FLOW;
            edge.currentTravelTime = edge.baseTravelTime;
            edge.isBlocked = false;
        }

        // Clear selections
        selectedStartNode = -1;
        selectedEndNode = -1;
        sourceText = "";
        destText = "";
        currentPath.clear();

        std::cout << "All cleared" << std::endl;
    }

    // Clear Accidents button
    else if (clearAccidentsBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Clear Accidents clicked" << std::endl;

        if (accidentSystem) {
            accidentSystem->clearAllAccidents();
            std::cout << "All accidents cleared" << std::endl;
        }
    }

    // Toggle Predictions button
    else if (togglePredictionsBtn.shape.getGlobalBounds().contains(screenPos)) {
        std::cout << "Predictions clicked - Toggling prediction overlay" << std::endl;

        showPredictions = !showPredictions;

        // Change button color to indicate state
        togglePredictionsBtn.shape.setFillColor(showPredictions ?
            sf::Color(100, 200, 100) : sf::Color(70, 130, 180));

        std::cout << "Predictions " << (showPredictions ? "enabled" : "disabled") << std::endl;
    }
}

void GUI::handleControlPanelClick(const sf::Vector2f& screenPos) {
    handleButtonClick(screenPos);

    if (sourceBox.getGlobalBounds().contains(screenPos)) {
        sourceActive = true;
        destActive = false;
    }
    else if (destBox.getGlobalBounds().contains(screenPos)) {
        destActive = true;
        sourceActive = false;
    }
    else {
        sourceActive = destActive = false;
    }
}

void GUI::handleMapClick(int x, int y) {
    sf::View mapView = window.getDefaultView();
    mapView.setSize(sf::Vector2f(UIConfig::MAP_VIEWPORT_WIDTH / zoomLevel, UIConfig::WINDOW_HEIGHT / zoomLevel));
    mapView.setCenter(sf::Vector2f(UIConfig::MAP_VIEWPORT_WIDTH / (2.0f * zoomLevel) + viewOffset.x,
        UIConfig::WINDOW_HEIGHT / (2.0f * zoomLevel) + viewOffset.y));

    sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(x, y), mapView);

    int nodeId = findNodeAtPosition(worldPos);
    
    if (nodeId != -1) {
        handleNodeSelection(nodeId);
    }
}

int GUI::findNodeAtPosition(const sf::Vector2f& worldPos) const {
    int nodeId = -1;
    float minDist = RenderConfig::NODE_SELECTION_RADIUS * zoomLevel;

    auto nodes = cityMap.getAllNodes();
    for (const auto& pair : nodes) {
        const Node& node = pair.second;
        float nodeX = node.x * zoomLevel + viewOffset.x;
        float nodeY = node.y * zoomLevel + viewOffset.y;

        float distance = std::sqrt(
            std::pow(nodeX - worldPos.x, 2) +
            std::pow(nodeY - worldPos.y, 2)
        );

        if (distance < minDist) {
            minDist = distance;
            nodeId = node.id;
        }
    }

    return nodeId;
}

void GUI::handleNodeSelection(int nodeId) {
    if (sourceActive) {
        sourceText = std::to_string(nodeId);
        selectedStartNode = nodeId;
    }
    else if (destActive) {
        destText = std::to_string(nodeId);
        selectedEndNode = nodeId;
    }
    else {
        if (selectedStartNode == -1) {
            selectedStartNode = nodeId;
            sourceText = std::to_string(nodeId);
        }
        else if (selectedEndNode == -1) {
            selectedEndNode = nodeId;
            destText = std::to_string(nodeId);
        }
        else {
            selectedStartNode = nodeId;
            selectedEndNode = -1;
            sourceText = std::to_string(nodeId);
            destText = "";
            currentPath.clear();
        }
    }
    std::cout << "Node " << nodeId << " selected" << std::endl;
}

void GUI::initializeFont() {
    bool fontLoaded = false;
    
    for (int i = 0; i < FontConfig::FONT_PATH_COUNT; ++i) {
        if (font.loadFromFile(FontConfig::FONT_PATHS[i])) {
            std::cout << "Font loaded from: " << FontConfig::FONT_PATHS[i] << std::endl;
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "Warning: Could not load font! Text rendering may fail." << std::endl;
    }
}

void GUI::initializeWarningTexture() {
    if (!warningTexture.loadFromFile(FileConfig::WARNING_TEXTURE_PATH)) {
        // Generate procedural warning texture
        sf::Image img;
        img.create(FileConfig::WARNING_TEXTURE_SIZE, FileConfig::WARNING_TEXTURE_SIZE, 
                   sf::Color::Transparent);

        const int size = FileConfig::WARNING_TEXTURE_SIZE;
        const int center = size / 2;
        
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (abs(x - center) <= y / 2 && y >= size / 4 && y <= 3 * size / 4) {
                    img.setPixel(x, y, sf::Color(255, 255, 0));
                }
            }
        }
        warningTexture.loadFromImage(img);
    }

    warningSprite.setTexture(warningTexture);
    warningSprite.setOrigin(FileConfig::WARNING_TEXTURE_SIZE / 2, 
                           FileConfig::WARNING_TEXTURE_SIZE / 2);
}

void GUI::initializeSystems() {
    std::cout << "Initializing subsystems..." << std::endl;

    predictionSystem = new PredictionSystem(&cityMap);
    if (!predictionSystem) {
        std::cerr << "ERROR: Failed to create PredictionSystem!" << std::endl;
    } else {
        std::cout << "  PredictionSystem initialized" << std::endl;
    }

    carSim = new CarSimulation(cityMap);
    if (!carSim) {
        std::cerr << "ERROR: Failed to create CarSimulation!" << std::endl;
    } else {
        std::cout << "  CarSimulation initialized" << std::endl;
    }

    accidentSystem = new AccidentSystem(&cityMap);
    if (!accidentSystem) {
        std::cerr << "ERROR: Failed to create AccidentSystem!" << std::endl;
    } else {
        std::cout << "  AccidentSystem initialized" << std::endl;
    }
}

void GUI::initializeUI() {
    initializeControlPanel();
    initializeInputFields();
    initializeButtons();
    initializeStatistics();
}

void GUI::initializeControlPanel() {
    controlPanel.setSize(sf::Vector2f(UIConfig::CONTROL_PANEL_WIDTH, UIConfig::WINDOW_HEIGHT));
    controlPanel.setFillColor(sf::Color(ColorConfig::PANEL_R, ColorConfig::PANEL_G, ColorConfig::PANEL_B));
    controlPanel.setPosition(UIConfig::CONTROL_PANEL_X, 0.0f);

    panelTitle.setFont(font);
    panelTitle.setString("Control Panel");
    panelTitle.setCharacterSize(UIConfig::PANEL_TITLE_SIZE);
    panelTitle.setFillColor(sf::Color::White);
    panelTitle.setStyle(sf::Text::Bold);
    panelTitle.setPosition(920.0f, 15.0f);
}

void GUI::initializeInputFields() {
    // Source input
    sourceLabel.setFont(font);
    sourceLabel.setString("Source Node:");
    sourceLabel.setCharacterSize(UIConfig::LABEL_SIZE);
    sourceLabel.setFillColor(sf::Color::White);
    sourceLabel.setPosition(920.0f, 60.0f);

    sourceBox.setSize(sf::Vector2f(UIConfig::INPUT_BOX_WIDTH, UIConfig::INPUT_BOX_HEIGHT));
    sourceBox.setFillColor(sf::Color(60, 60, 70));
    sourceBox.setOutlineColor(sf::Color(150, 150, 150));
    sourceBox.setOutlineThickness(1.5f);
    sourceBox.setPosition(920.0f, 85.0f);

    // Destination input
    destLabel.setFont(font);
    destLabel.setString("Destination:");
    destLabel.setCharacterSize(UIConfig::LABEL_SIZE);
    destLabel.setFillColor(sf::Color::White);
    destLabel.setPosition(920.0f, 125.0f);

    destBox.setSize(sf::Vector2f(UIConfig::INPUT_BOX_WIDTH, UIConfig::INPUT_BOX_HEIGHT));
    destBox.setFillColor(sf::Color(60, 60, 70));
    destBox.setOutlineColor(sf::Color(150, 150, 150));
    destBox.setOutlineThickness(1.5f);
    destBox.setPosition(920.0f, 150.0f);
}

void GUI::initializeButtons() {
    const float col1X = 920.0f;
    const float col2X = col1X + UIConfig::BUTTON_WIDTH + UIConfig::COLUMN_SPACING;
    const float startY = UIConfig::BUTTON_START_Y;
    const float spacing = UIConfig::BUTTON_SPACING;

    // Column 1 (left)
    createButton(findRouteBtn, col1X, startY, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Find Path");
    createButton(addCarBtn, col1X, startY + spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Add Car");
    createButton(clearCarsBtn, col1X, startY + 2 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Clear Cars");
    createButton(trafficBtn, col1X, startY + 3 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Traffic Sim");
    createButton(peakHourBtn, col1X, startY + 4 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Peak Hour");
    createButton(accidentBtn, col1X, startY + 5 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Accident");

    // Column 2 (right)
    createButton(generateCityBtn, col2X, startY, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Generate City");
    createButton(spawnManyCarsBtn, col2X, startY + spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "20 Cars");
    createButton(rushHourBtn, col2X, startY + 2 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Rush Hour");
    createButton(clearTrafficBtn, col2X, startY + 3 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Clear All");
    createButton(clearAccidentsBtn, col2X, startY + 4 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Clear Accidents");
    createButton(togglePredictionsBtn, col2X, startY + 5 * spacing, UIConfig::BUTTON_WIDTH, UIConfig::BUTTON_HEIGHT, "Predictions");
}

void GUI::initializeStatistics() {
    float statsStartY = UIConfig::BUTTON_START_Y + 6 * UIConfig::BUTTON_SPACING + 10.0f;
    statsText.setFont(font);
    statsText.setCharacterSize(UIConfig::STATS_TEXT_SIZE);
    statsText.setFillColor(sf::Color::White);
    statsText.setLineSpacing(0.8f);
    statsText.setPosition(920.0f, statsStartY);
    updateStatistics(0, 0, 0, 0.0f);
}

void GUI::handleTextInput(sf::Uint32 unicode) {
    if (unicode == InputConfig::BACKSPACE_KEY) {
        handleBackspace();
    }
    else if (unicode >= InputConfig::DIGIT_START && unicode <= InputConfig::DIGIT_END) {
        handleDigitInput(static_cast<char>(unicode));
    }
    else if (unicode == InputConfig::ENTER_KEY) {
        handleEnterKey();
    }
}

void GUI::handleBackspace() {
    if (sourceActive && !sourceText.empty()) {
        sourceText.pop_back();
    }
    else if (destActive && !destText.empty()) {
        destText.pop_back();
    }
}

void GUI::handleDigitInput(char digit) {
    if (sourceActive) {
        sourceText += digit;
        try {
            selectedStartNode = std::stoi(sourceText);
        }
        catch (const std::exception&) {
            selectedStartNode = -1;
            std::cerr << "Invalid source node ID" << std::endl;
        }
    }
    else if (destActive) {
        destText += digit;
        try {
            selectedEndNode = std::stoi(destText);
        }
        catch (const std::exception&) {
            selectedEndNode = -1;
            std::cerr << "Invalid destination node ID" << std::endl;
        }
    }
}

void GUI::handleEnterKey() {
    if (sourceActive) {
        sourceActive = false;
        destActive = true;
    }
    else if (destActive) {
        destActive = false;
        sourceActive = true;
    }
}

void GUI::createButton(Button& btn, float x, float y, float w, float h, const std::string& text) {
    btn.shape.setSize(sf::Vector2f(w, h));
    btn.shape.setPosition(x, y);
    btn.shape.setFillColor(sf::Color(70, 130, 180));
    btn.shape.setOutlineColor(sf::Color::White);
    btn.shape.setOutlineThickness(1.0f);

    btn.label.setFont(font);
    btn.label.setString(text);
    btn.label.setCharacterSize(UIConfig::BUTTON_TEXT_SIZE);
    btn.label.setFillColor(sf::Color::White);
    btn.label.setStyle(sf::Text::Bold);

    sf::FloatRect textBounds = btn.label.getLocalBounds();
    btn.label.setOrigin(0, 0);
    float textX = x + (w - textBounds.width) / 2.0f;
    float textY = y + (h - textBounds.height) / 2.0f - 4.0f;
    btn.label.setPosition(textX, textY);
}

void GUI::drawButton(const Button& btn) {
    window.draw(btn.shape);
    window.draw(btn.label);
}

void GUI::setCurrentPath(const std::vector<int>& path) {
    currentPath = path;
}

void GUI::updateAccidentVisuals() {
    accidentIcons.clear();

    if (!accidentSystem) return;

    auto accidentEdges = accidentSystem->getAccidentEdges();

    for (int edgeId : accidentEdges) {
        Edge edge = cityMap.getEdge(edgeId);
        if (edge.id == -1) continue;

        Node fromNode = cityMap.getNode(edge.fromNodeId);
        Node toNode = cityMap.getNode(edge.toNodeId);

        float midX = (fromNode.x + toNode.x) / 2.0f;
        float midY = (fromNode.y + toNode.y) / 2.0f;

        float screenX = midX * zoomLevel + viewOffset.x;
        float screenY = midY * zoomLevel + viewOffset.y - 20.0f; 

        sf::Sprite icon = warningSprite;
        icon.setPosition(screenX, screenY);
        icon.setScale(0.5f * zoomLevel, 0.5f * zoomLevel);

        float pulse = sin(icon.getPosition().x * 0.1f + guiClock.getElapsedTime().asSeconds() * 3.0f) * 0.2f + 1.0f;
        icon.setScale(0.5f * zoomLevel * pulse, 0.5f * zoomLevel * pulse);

        accidentIcons.push_back(icon);
    }
}

void GUI::updateStatistics(int nodeCount, int edgeCount, int carCount, float avgSpeed) {
    std::stringstream ss;

    // Current stats
    ss << "=== LIVE STATISTICS ===\n";
    ss << "Nodes:      " << std::setw(4) << nodeCount << "\n";
    ss << "Roads:      " << std::setw(4) << edgeCount << "\n";
    ss << "Active Cars:" << std::setw(4) << carCount << "\n";
    ss << "Avg Speed:  " << std::setw(4) << std::fixed << std::setprecision(1)
        << avgSpeed << " km/h\n";

    int congestedRoads = 0;
    auto edges = cityMap.getAllEdges();
    for (const auto& pair : edges) {
        if (pair.second.trafficLevel == TrafficLevel::CONGESTED ||
            pair.second.trafficLevel == TrafficLevel::BLOCKED) {
            congestedRoads++;
        }
    }

    float congestionPercent = (edges.size() > 0) ?
        (congestedRoads * 100.0f / edges.size()) : 0.0f;

    ss << "Congestion: " << std::setw(4) << std::fixed << std::setprecision(1)
        << congestionPercent << "%\n";

    if (selectedStartNode != -1 && selectedEndNode != -1) {
        auto path = cityMap.findShortestPath(selectedStartNode, selectedEndNode);
        if (!path.empty()) {
            float travelTime = 0.0f;
            for (size_t i = 0; i < path.size() - 1; i++) {
                int fromNode = path[i];
                int toNode = path[i + 1];

                int edgeId = cityMap.findEdgeId(fromNode, toNode);
                if (edgeId != -1) {
                    Edge edge = cityMap.getEdge(edgeId);
                    travelTime += edge.currentTravelTime;
                }
            }
            ss << "Est. Time:  " << std::setw(4) << std::fixed << std::setprecision(1)
                << travelTime << " min\n";
        }
    }


    int accidentCount = 0;
    std::vector<int> accidentEdges;

    if (accidentSystem) {
        accidentCount = accidentSystem->getActiveAccidentCount();
        accidentEdges = accidentSystem->getAccidentEdges();
    }

    ss << "Accidents:  " << std::setw(4) << accidentCount << "\n";


    int predictedCongestion = 0;
    float predictionAccuracy = 0.0f;

    if (predictionSystem) {
        predictedCongestion = predictionSystem->getPredictedCongestionCount();
        predictionAccuracy = predictionSystem->getAveragePredictionAccuracy();
    }
    else {
        std::cout << "ERROR: predictionSystem is NULL in updateStatistics!" << std::endl;
    }

    ss << "Pred. Cong: " << std::setw(4) << predictedCongestion << "\n";
    ss << "Pred. Acc:  " << std::setw(4) << std::fixed << std::setprecision(1)
        << (predictionAccuracy * 100.0f) << "%\n";


    if (accidentCount > 0) {
        ss << "Affected Roads: ";
        for (size_t i = 0; i < std::min(accidentEdges.size(), (size_t)3); i++) {
            ss << accidentEdges[i];
            if (i < std::min(accidentEdges.size(), (size_t)3) - 1) {
                ss << ", ";
            }
        }
        if (accidentEdges.size() > 3) {
            ss << "...";
        }
        ss << "\n";
    }


    ss << "\n=== SELECTION ===\n";
    ss << "From: Node " << (selectedStartNode == -1 ? "---" : std::to_string(selectedStartNode)) << "\n";
    ss << "To:   Node " << (selectedEndNode == -1 ? "---" : std::to_string(selectedEndNode));

    statsText.setString(ss.str());
}

void GUI::addCar(int startNode, int endNode) {
    std::cout << "Adding car from " << startNode << " to " << endNode << std::endl;

    if (carSim && startNode != endNode) {
        std::vector<int> path = cityMap.findShortestPath(startNode, endNode);
        if (!path.empty()) {
            carSim->addCar(startNode, endNode, path);
            totalCarsSpawned++;
        }
    }
}