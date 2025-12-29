#include "MapRenderer.h"
#include <iostream>
#include <cmath>

MapRenderer::MapRenderer() {
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font. Using default." << std::endl;
    }
    nodeText.setFont(font);
    nodeText.setCharacterSize(12);
    edgeText.setFont(font);
    edgeText.setCharacterSize(10);
}

bool MapRenderer::loadFont(const std::string& fontPath) {
    return font.loadFromFile(fontPath);
}

void MapRenderer::drawGraph(sf::RenderWindow& window, const Graph& graph,
    float zoom, sf::Vector2f offset) {
    for (const auto& [id, edge] : graph.getAllEdges()) {
        drawEdge(window, edge, graph, zoom, offset);
    }

    for (const auto& [id, node] : graph.getAllNodes()) {
        drawNode(window, node, false, zoom, offset);
    }
}

void MapRenderer::drawNode(sf::RenderWindow& window, const Node& node,
    bool isSelected, float zoom, sf::Vector2f offset) {
    float screenX = node.x * zoom + offset.x;
    float screenY = node.y * zoom + offset.y;

    sf::CircleShape circle(8.0f * zoom);
    circle.setPosition(screenX - 8.0f * zoom, screenY - 8.0f * zoom);

    if (isSelected) {
        circle.setFillColor(sf::Color::Cyan);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(2.0f * zoom);
    }
    else {
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(sf::Color(100, 100, 100));
        circle.setOutlineThickness(1.0f * zoom);
    }

    window.draw(circle);

    nodeText.setString(std::to_string(node.id));
    nodeText.setCharacterSize(static_cast<unsigned int>(12 * zoom));
    nodeText.setFillColor(sf::Color::Black);
    nodeText.setPosition(screenX - 5.0f * zoom, screenY - 6.0f * zoom);
    window.draw(nodeText);
}

void MapRenderer::drawEdge(sf::RenderWindow& window, const Edge& edge,
    const Graph& graph, float zoom, sf::Vector2f offset) {
    Node fromNode = graph.getNode(edge.fromNodeId);
    Node toNode = graph.getNode(edge.toNodeId);

    float fromX = fromNode.x * zoom + offset.x;
    float fromY = fromNode.y * zoom + offset.y;
    float toX = toNode.x * zoom + offset.x;
    float toY = toNode.y * zoom + offset.y;

    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(fromX, fromY)),
        sf::Vertex(sf::Vector2f(toX, toY))
    };

    line[0].color = getTrafficColor(edge.trafficLevel);
    line[1].color = getTrafficColor(edge.trafficLevel);

    for (int i = -1; i <= 1; i++) {
        line[0].position = sf::Vector2f(fromX + i, fromY + i);
        line[1].position = sf::Vector2f(toX + i, toY + i);
        window.draw(line, 2, sf::Lines);
    }

    if (zoom > 0.8f) {
        edgeText.setString(edge.name);
        edgeText.setCharacterSize(static_cast<unsigned int>(10 * zoom));
        edgeText.setFillColor(sf::Color(150, 150, 150));
        edgeText.setPosition((fromX + toX) / 2.0f - 20.0f * zoom,
            (fromY + toY) / 2.0f - 10.0f * zoom);
        window.draw(edgeText);
    }
}

void MapRenderer::drawRoute(sf::RenderWindow& window, const std::vector<int>& nodePath,
    const Graph& graph, float zoom, sf::Vector2f offset) {
    if (nodePath.size() < 2) return;

    for (size_t i = 0; i < nodePath.size() - 1; i++) {
        Node fromNode = graph.getNode(nodePath[i]);
        Node toNode = graph.getNode(nodePath[i + 1]);

        float fromX = fromNode.x * zoom + offset.x;
        float fromY = fromNode.y * zoom + offset.y;
        float toX = toNode.x * zoom + offset.x;
        float toY = toNode.y * zoom + offset.y;

        // Draw route line
        sf::Vertex routeLine[] = {
            sf::Vertex(sf::Vector2f(fromX, fromY), sf::Color(0, 255, 255, 200)),
            sf::Vertex(sf::Vector2f(toX, toY), sf::Color(0, 255, 255, 200))
        };

        // Draw thicker line for route
        for (int j = -2; j <= 2; j++) {
            routeLine[0].position = sf::Vector2f(fromX + j, fromY + j);
            routeLine[1].position = sf::Vector2f(toX + j, toY + j);
            window.draw(routeLine, 2, sf::Lines);
        }
    }
}

sf::Color MapRenderer::getTrafficColor(TrafficLevel level) const {
    switch (level) {
    case TrafficLevel::FREE_FLOW: return sf::Color::Green;
    case TrafficLevel::SLOW: return sf::Color::Yellow;
    case TrafficLevel::CONGESTED: return sf::Color::Red;
    case TrafficLevel::BLOCKED: return sf::Color(100, 100, 100);
    default: return sf::Color::White;
    }
}