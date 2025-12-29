#include "UIManager.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <SFML/Window/VideoMode.hpp>

UIManager::UIManager(const Graph& g)
    : window(sf::VideoMode(1200, 800), "Traffic Analyzer"),
    graph(g),
    mapRenderer(),
    carSim(g),
    showCars(true),
    selectedStartNode(-1),
    selectedEndNode(-1),
    zoomLevel(1.0f),
    viewOffset(0.0f, 0.0f),
    isDragging(false),
    lastMousePos(0, 0),
    font(),
    statusText(),
    infoPanel(),
    legendText(),
    simulateTraffic(false),
    trafficUpdateTimer(0.0f) {

    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
    }

    statusText.setFont(font);
    statusText.setCharacterSize(16);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(10.0f, 10.0f);

    infoPanel.setSize(sf::Vector2f(300.0f, 800.0f));
    infoPanel.setFillColor(sf::Color(40, 40, 50, 220));
    infoPanel.setPosition(900.0f, 0.0f);

    legendText.setFont(font);
    legendText.setCharacterSize(14);
    legendText.setFillColor(sf::Color::White);
    legendText.setPosition(910.0f, 50.0f);

    // Add sample vehicles (placeholder)
    addVehicle(1, 6);
}

void UIManager::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        handleEvents();
        update(deltaTime);
        render();
    }
}

void UIManager::handleEvents() {
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

        case sf::Event::KeyPressed:
            handleKeyPress(event.key.code);
            break;

        default:
            break;
        }
    }
}

void UIManager::update(float deltaTime) {
    // Update car simulation (if enabled)
    if (showCars) {
        carSim.update(deltaTime);
    }

    // Update traffic simulation
    if (simulateTraffic) {
        trafficUpdateTimer += deltaTime;
        if (trafficUpdateTimer >= 2.0f) {
            trafficUpdateTimer = 0.0f;
            // Would call traffic updates here
        }
    }

    // Update status text
    std::stringstream ss;
    ss << "Traffic Analyzer v1.0\n";
    ss << "Nodes: " << graph.getNodeCount() << " | Roads: " << graph.getEdgeCount() << "\n";
    ss << "Start: " << (selectedStartNode == -1 ? "None" : std::to_string(selectedStartNode)) << "\n";
    ss << "End: " << (selectedEndNode == -1 ? "None" : std::to_string(selectedEndNode)) << "\n";
    ss << "Traffic Sim: " << (simulateTraffic ? "ON" : "OFF");
    statusText.setString(ss.str());

    // Update legend
    std::stringstream legend;
    legend << "TRAFFIC LEGEND:\n";
    legend << "Green: Free Flow\n";
    legend << "Yellow: Slow\n";
    legend << "Red: Congested\n";
    legend << "Gray: Blocked\n";
    legend << "\nCONTROLS:\n";
    legend << "L-Click: Select nodes\n";
    legend << "R-Click: Pan view\n";
    legend << "Wheel: Zoom\n";
    legend << "R: Calculate route\n";
    legend << "C: Add car\n";
    legend << "T: Toggle traffic\n";
    legend << "ESC: Exit";
    legendText.setString(legend.str());
}

void UIManager::render() {
    window.clear(sf::Color(30, 30, 40));

    // Apply view transformation
    sf::View view = window.getDefaultView();
    view.setSize(sf::Vector2f(1200.0f / zoomLevel, 800.0f / zoomLevel));
    view.setCenter(sf::Vector2f(600.0f / zoomLevel + viewOffset.x,
        400.0f / zoomLevel + viewOffset.y));
    window.setView(view);

    // Draw map
    mapRenderer.drawGraph(window, graph, zoomLevel, viewOffset);

    // Draw route if exists
    if (!currentRoute.empty()) {
        mapRenderer.drawRoute(window, currentRoute, graph, zoomLevel, viewOffset);
    }

    // Draw cars
    if (showCars) {
        carSim.draw(window, zoomLevel, viewOffset);
    }

    // Reset view for UI
    window.setView(window.getDefaultView());
    drawUI();

    window.display();
}

void UIManager::drawUI() {
    window.draw(infoPanel);
    window.draw(statusText);
    window.draw(legendText);
}

void UIManager::handleMouseClick(int x, int y) {
    int nodeId = getNodeAtPosition(x, y);
    if (nodeId != -1) {
        if (selectedStartNode == -1) {
            selectedStartNode = nodeId;
            std::cout << "Selected start node: " << nodeId << std::endl;
        }
        else if (selectedEndNode == -1) {
            selectedEndNode = nodeId;
            std::cout << "Selected end node: " << nodeId << std::endl;
        }
        else {
            selectedStartNode = nodeId;
            selectedEndNode = -1;
            currentRoute.clear();
        }
    }
}

void UIManager::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
    case sf::Keyboard::R:
        calculateRoute();
        break;
    case sf::Keyboard::C:
        if (selectedStartNode != -1 && selectedEndNode != -1) {
            addVehicle(selectedStartNode, selectedEndNode);
        }
        break;
    case sf::Keyboard::T:
        toggleTrafficSimulation();
        break;
    case sf::Keyboard::Escape:
        window.close();
        break;
    default:
        break;
    }
}

sf::Vector2f UIManager::screenToWorld(int x, int y) const {
    sf::View view = window.getDefaultView();
    view.setSize(sf::Vector2f(1200.0f / zoomLevel, 800.0f / zoomLevel));
    view.setCenter(sf::Vector2f(600.0f / zoomLevel + viewOffset.x,
        400.0f / zoomLevel + viewOffset.y));
    return window.mapPixelToCoords(sf::Vector2i(x, y), view);
}

int UIManager::getNodeAtPosition(int x, int y) const {
    sf::Vector2f worldPos = screenToWorld(x, y);

    for (const auto& [id, node] : graph.getAllNodes()) {
        float nodeX = node.x * zoomLevel + viewOffset.x;
        float nodeY = node.y * zoomLevel + viewOffset.y;

        float distance = std::sqrt(
            std::pow(nodeX - worldPos.x, 2) +
            std::pow(nodeY - worldPos.y, 2)
        );

        if (distance < 15.0f) {
            return id;
        }
    }
    return -1;
}

void UIManager::addVehicle(int startNode, int endNode) {
    std::cout << "Vehicle added from " << startNode << " to " << endNode << std::endl;
    // Vehicle logic would go here (or forward to carSim)
    // Example: carSim.addCar(startNode, endNode, std::vector<int>{startNode, endNode});
}

void UIManager::addCarToSimulation(int start, int end)
{
    // Use the graph to calculate the route between start and end nodes
    std::vector<int> route = graph.findShortestPath(start, end);
    // Pass the route to carSim.addCar as required by its signature
    carSim.addCar(start, end, route);
}

void UIManager::calculateRoute() {
    if (selectedStartNode == -1 || selectedEndNode == -1) {
        std::cout << "Please select both start and end nodes" << std::endl;
        return;
    }

    currentRoute = { selectedStartNode, selectedEndNode };
    std::cout << "Calculated route from " << selectedStartNode
        << " to " << selectedEndNode << std::endl;
}

void UIManager::toggleTrafficSimulation() {
    simulateTraffic = !simulateTraffic;
    std::cout << "Traffic simulation: " << (simulateTraffic ? "ON" : "OFF") << std::endl;
}