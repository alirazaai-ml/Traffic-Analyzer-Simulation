#include "GUI.h"
#include "Graph.h"
#include "CarSimulation.h"
#include "MapGenerator.h"
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
    window(sf::VideoMode(1200, 800), "Traffic Analysis System"),
    zoomLevel(1.0f),
    viewOffset(0.0f, 0.0f),
    isDragging(false),
    selectedStartNode(-1),
    selectedEndNode(-1),
    sourceActive(false),
    destActive(false),
    carSim(nullptr),
    showCars(true),
    simulationSpeed(1.0f),
    totalCarsSpawned(0),
    freeFlowColor(0, 200, 0),
    slowColor(255, 255, 0),
    congestedColor(255, 50, 50),
    blockedColor(100, 100, 100),
    showPredictions(false),
    predictedCongestionColor(128, 0, 128)  
{
    std::cout << "Initializing GUI..." << std::endl;

    // Load font
    bool fontLoaded = false;
    const char* fontPaths[] = {
        "arial.ttf",
        "fonts/arial.ttf",
        "../fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/calibri.ttf"
    };

    for (const char* path : fontPaths) {
        if (font.loadFromFile(path)) {
            std::cout << "Font loaded from: " << path << std::endl;
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "Warning: Could not load font!" << std::endl;
    }


    // Load warning texture or create one
    if (!warningTexture.loadFromFile("warning.png")) {
        sf::Image img;
        img.create(32, 32, sf::Color::Transparent);

        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                if (abs(x - 16) <= y / 2 && y >= 8 && y <= 24) {
                    img.setPixel(x, y, sf::Color(255, 255, 0));
                }
            }
        }
        warningTexture.loadFromImage(img);
    }

    warningSprite.setTexture(warningTexture);
    warningSprite.setOrigin(16, 16); 

    std::cout << "Initializing PredictionSystem..." << std::endl;
    predictionSystem = new PredictionSystem(&cityMap);

    if (predictionSystem) {
        std::cout << "PredictionSystem created successfully!" << std::endl;
    }
    else {
        std::cout << "ERROR: Failed to create PredictionSystem!" << std::endl;
    }

    carSim = new CarSimulation(map);

    accidentSystem = new AccidentSystem(&cityMap);

    // Setup Control Panel
    controlPanel.setSize(sf::Vector2f(300.0f, 800.0f));
    controlPanel.setFillColor(sf::Color(40, 40, 50));
    controlPanel.setPosition(900.0f, 0.0f);

    // Panel Title
    panelTitle.setFont(font);
    panelTitle.setString("Control Panel");
    panelTitle.setCharacterSize(20); 
    panelTitle.setFillColor(sf::Color::White);
    panelTitle.setStyle(sf::Text::Bold);
    panelTitle.setPosition(920.0f, 15.0f);

    // Source input
    sourceLabel.setFont(font);
    sourceLabel.setString("Source Node:");
    sourceLabel.setCharacterSize(14); 
    sourceLabel.setFillColor(sf::Color::White);
    sourceLabel.setPosition(920.0f, 60.0f); 

    sourceBox.setSize(sf::Vector2f(200.0f, 25.0f)); 
    sourceBox.setFillColor(sf::Color(60, 60, 70));
    sourceBox.setOutlineColor(sf::Color(150, 150, 150));
    sourceBox.setOutlineThickness(1.5f); 
    sourceBox.setPosition(920.0f, 85.0f); 

    // Destination input
    destLabel.setFont(font);
    destLabel.setString("Destination:");
    destLabel.setCharacterSize(14); 
    destLabel.setFillColor(sf::Color::White);
    destLabel.setPosition(920.0f, 125.0f); 

    destBox.setSize(sf::Vector2f(200.0f, 25.0f)); 
    destBox.setFillColor(sf::Color(60, 60, 70));
    destBox.setOutlineColor(sf::Color(150, 150, 150));
    destBox.setOutlineThickness(1.5f); 
    destBox.setPosition(920.0f, 150.0f);


    float buttonWidth = 130.0f; 
    float buttonHeight = 28.0f;
    float startY = 190.0f;
    float buttonSpacing = 32.0f;
    float columnSpacing = 5.0f; 

    // COLUMN 1 (left)
    createButton(findRouteBtn, 920.0f, startY, buttonWidth, buttonHeight, "Find Path");
    createButton(addCarBtn, 920.0f, startY + buttonSpacing, buttonWidth, buttonHeight, "Add Car");
    createButton(clearCarsBtn, 920.0f, startY + 2 * buttonSpacing, buttonWidth, buttonHeight, "Clear Cars");
    createButton(trafficBtn, 920.0f, startY + 3 * buttonSpacing, buttonWidth, buttonHeight, "Traffic Sim");
    createButton(peakHourBtn, 920.0f, startY + 4 * buttonSpacing, buttonWidth, buttonHeight, "Peak Hour");
    createButton(accidentBtn, 920.0f, startY + 5 * buttonSpacing, buttonWidth, buttonHeight, "Accident");

    // COLUMN 2 (right - shifted 5px)
    createButton(generateCityBtn, 920.0f + buttonWidth + columnSpacing, startY, buttonWidth, buttonHeight, "Generate City");
    createButton(spawnManyCarsBtn, 920.0f + buttonWidth + columnSpacing, startY + buttonSpacing, buttonWidth, buttonHeight, "20 Cars");
    createButton(rushHourBtn, 920.0f + buttonWidth + columnSpacing, startY + 2 * buttonSpacing, buttonWidth, buttonHeight, "Rush Hour");
    createButton(clearTrafficBtn, 920.0f + buttonWidth + columnSpacing, startY + 3 * buttonSpacing, buttonWidth, buttonHeight, "Clear All");
    createButton(clearAccidentsBtn, 920.0f + buttonWidth + columnSpacing,
        startY + 4 * buttonSpacing, buttonWidth, buttonHeight, "Clear Accidents");
    createButton(togglePredictionsBtn, 920.0f + buttonWidth + columnSpacing,
        startY + 5 * buttonSpacing, buttonWidth, buttonHeight, "Predictions");

    // Statistics text - BELOW both columns
    float statsStartY = startY + 6 * buttonSpacing + 10.0f; 
    statsText.setFont(font);
    statsText.setCharacterSize(11); 
    statsText.setFillColor(sf::Color::White);
    statsText.setLineSpacing(0.8f);
    statsText.setPosition(920.0f, statsStartY);

    updateStatistics(0, 0, 0, 0.0f);
}

// Destructor
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

    float baseWidth = 3.0f * zoomLevel;

    if (carSim) {
        int carsOnThisEdge = 0; 

        if (carsOnThisEdge > 3) {
            baseWidth = 5.0f * zoomLevel;
        }
        else if (carsOnThisEdge > 1) {
            baseWidth = 4.0f * zoomLevel;
        }
    }

    // Use baseWidth for road thickness
    //road.setSize(sf::Vector2f(length, baseWidth));


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

    if (screenPos.x >= 900.0f) {
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
        return;
    }

    sf::View mapView = window.getDefaultView();
    mapView.setSize(sf::Vector2f(900.0f / zoomLevel, 800.0f / zoomLevel));
    mapView.setCenter(sf::Vector2f(450.0f / zoomLevel + viewOffset.x,
        400.0f / zoomLevel + viewOffset.y));

    sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(x, y), mapView);

    // Find node at this position
    int nodeId = -1;
    float minDist = 15.0f * zoomLevel; 

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

    if (nodeId != -1) {
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
}

void GUI::handleTextInput(sf::Uint32 unicode) {
    if (unicode == 8) { 
        if (sourceActive && !sourceText.empty()) {
            sourceText.pop_back();
        }
        else if (destActive && !destText.empty()) {
            destText.pop_back();
        }
    }
    else if (unicode >= 48 && unicode <= 57) { 
        if (sourceActive) {
            sourceText += static_cast<char>(unicode);
            try {
                selectedStartNode = std::stoi(sourceText);
            }
            catch (...) {
                selectedStartNode = -1;
            }
        }
        else if (destActive) {
            destText += static_cast<char>(unicode);
            try {
                selectedEndNode = std::stoi(destText);
            }
            catch (...) {
                selectedEndNode = -1;
            }
        }
    }
    else if (unicode == 13) { 
        if (sourceActive) {
            sourceActive = false;
            destActive = true;
        }
        else if (destActive) {
            destActive = false;
            sourceActive = true;
        }
    }
}

void GUI::handleButtonClick(const sf::Vector2f& mousePos) {
    if (findRouteBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Find Shortest Path button clicked!" << std::endl;

        if (selectedStartNode != -1 && selectedEndNode != -1) {
            std::vector<int> path = cityMap.findShortestPath(selectedStartNode, selectedEndNode);
            if (!path.empty()) {
                currentPath = path;
                std::cout << "Path found with " << path.size() << " nodes" << std::endl;
            }
            else {
                std::cout << "No path found!" << std::endl;
                currentPath.clear();
            }
        }
        else {
            std::cout << "Please select both source and destination nodes!" << std::endl;
        }
    }
    else if (addCarBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Add Random Car button clicked!" << std::endl;

        auto nodes = cityMap.getAllNodes();
        if (nodes.size() >= 2) {
            std::vector<int> nodeIds;
            for (const auto& pair : nodes) {
                nodeIds.push_back(pair.first);
            }

            int startNode = nodeIds[rand() % nodeIds.size()];
            int endNode = nodeIds[rand() % nodeIds.size()];

            while (endNode == startNode && nodeIds.size() > 1) {
                endNode = nodeIds[rand() % nodeIds.size()];
            }

            addCar(startNode, endNode);
        }
    }
    else if (clearCarsBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Clear Cars button clicked!" << std::endl;
        if (carSim) {
            carSim->clearAllCars();
        }
        currentPath.clear();
    }
    else if (trafficBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "\n TRAFFIC SIMULATION BUTTON CLICKED" << std::endl;

        if (carSim) {
            carSim->toggleRunning();

            if (carSim->getIsRunning()) {
                trafficBtn.label.setString("Stop Traffic");
                std::cout << " TRAFFIC SIMULATION ACTIVE" << std::endl;
                std::cout << "   Cars will spawn automatically" << std::endl;
            }
            else {
                trafficBtn.label.setString("Traffic Sim");
                std::cout << "TRAFFIC SIMULATION INACTIVE" << std::endl;
            }
        }
    }
    else if (peakHourBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "\n  PEAK HOUR SIMULATION" << std::endl;

        if (carSim) {
            for (int i = 0; i < 30; i++) {
                carSim->addRandomCar();
            }
        }

        auto edges = cityMap.getAllEdges();
        for (const auto& pair : edges) {
            Edge edge = pair.second;

            Node fromNode = cityMap.getNode(edge.fromNodeId);
            Node toNode = cityMap.getNode(edge.toNodeId);
            float midX = (fromNode.x + toNode.x) / 2;
            float midY = (fromNode.y + toNode.y) / 2;

            if (abs(midX - 400) < 200 && abs(midY - 300) < 200) {
                cityMap.updateEdgeTraffic(pair.first, edge.speedLimit * 0.3f);
            }
        }

        peakHourBtn.shape.setFillColor(sf::Color(255, 100, 100));
        peakHourBtn.label.setString("Peak Active");

        std::cout << " Added 30 cars and congested downtown!" << std::endl;
    }
    
    else if (accidentBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Random Accident button clicked!" << std::endl;

        auto edges = cityMap.getAllEdges();
        if (!edges.empty()) {
            auto it = edges.begin();
            std::advance(it, rand() % edges.size());
            int randomEdgeId = it->first;

            if (accidentSystem) {
                accidentSystem->createAccident(randomEdgeId, 180.0f); 
            }

            cityMap.blockEdge(randomEdgeId, 180.0f);

            std::cout << "Accident created on edge " << randomEdgeId << std::endl;
        }
    }
    else if (generateCityBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "\n🎮 GENERATE CITY BUTTON CLICKED" << std::endl;

        MapGenerator::generateNextCity(cityMap);

        if (carSim) {
            carSim->clearAllCars();
        }

        selectedStartNode = -1;
        selectedEndNode = -1;
        sourceText = "";
        destText = "";
        currentPath.clear();

        zoomLevel = 1.0f;
        viewOffset = sf::Vector2f(0.0f, 0.0f);

        std::string cityNames[6] = {
            "Simple Grid", "Complex City", "Random City",
            "Metropolis", "Planned City", "Coastal City"
        };

        static int currentType = 0;
        currentType = (currentType + 1) % 6;

        generateCityBtn.label.setString("City: " + cityNames[currentType]);

        std::cout << "Current city: " << cityNames[currentType] << "\n" << std::endl;
    }
    else if (spawnManyCarsBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Add 20 Cars button clicked!" << std::endl;

        if (carSim) {
            auto nodes = cityMap.getAllNodes();
            if (nodes.size() >= 2) {
                std::vector<int> nodeIds;
                for (const auto& pair : nodes) {
                    nodeIds.push_back(pair.first);
                }

                for (int i = 0; i < 20; i++) {
                    int startNode = nodeIds[rand() % nodeIds.size()];
                    int endNode = nodeIds[rand() % nodeIds.size()];

                    while (endNode == startNode && nodeIds.size() > 1) {
                        endNode = nodeIds[rand() % nodeIds.size()];
                    }

                    addCar(startNode, endNode);
                }
                std::cout << "Added 20 cars!" << std::endl;
            }
        }
    }

    else if (rushHourBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "\n SIMULATING RUSH HOUR TRAFFIC JAM" << std::endl;

        if (carSim) {
            for (int i = 0; i < 40; i++) {
                carSim->addRandomCar();
            }
        }

        auto edges = cityMap.getAllEdges();
        if (!edges.empty() && accidentSystem) {
            auto it = edges.begin();
            std::advance(it, rand() % edges.size());
            int edgeId = it->first;

            accidentSystem->createAccident(edgeId, 180.0f);
            cityMap.blockEdge(edgeId, 180.0f);

            std::cout << "   Accident on road " << edgeId << " (3 minutes)" << std::endl;
        }

        simulationSpeed = 0.3f;

        rushHourBtn.shape.setFillColor(sf::Color(255, 50, 50));

        std::cout << " Created traffic jam! 40 cars + accident" << std::endl;
        }
    else if (clearTrafficBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Clearing traffic..." << std::endl;

        auto edges = cityMap.getAllEdges();
        for (const auto& pair : edges) {
            cityMap.updateEdgeTraffic(pair.first, pair.second.speedLimit);
        }

        if (carSim) {
            carSim->clearAllCars();
        }

        simulationSpeed = 1.0f;
    }
    else if (clearAccidentsBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Clear Accidents button clicked!" << std::endl;

        if (accidentSystem) {
            accidentSystem->clearAllAccidents();
        }

        auto edges = cityMap.getAllEdges();
        for (const auto& pair : edges) {
            cityMap.unblockEdge(pair.first);
        }
    }
    else if (togglePredictionsBtn.shape.getGlobalBounds().contains(mousePos)) {
        std::cout << "Toggle Predictions button clicked!" << std::endl;

        showPredictions = !showPredictions;
        std::cout << "Predictions display: " << (showPredictions ? "ON" : "OFF") << std::endl;

        if (showPredictions && predictionSystem) {
            std::cout << "Predicting congestion for next 5 minutes..." << std::endl;
            auto congestedEdges = predictionSystem->getEdgesLikelyToCongest(5);
            std::cout << "Found " << congestedEdges.size() << " edges likely to congest" << std::endl;
        }
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
    btn.label.setCharacterSize(13); 
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