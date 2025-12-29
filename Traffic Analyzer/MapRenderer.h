#pragma once
#include "Graph.h"
#include <SFML/Graphics.hpp>

class MapRenderer {
private:
    sf::Font font;
    sf::Text nodeText;
    sf::Text edgeText;

public:
    MapRenderer();
    bool loadFont(const std::string& fontPath = "arial.ttf");

    void drawGraph(sf::RenderWindow& window, const Graph& graph,
        float zoom = 1.0f, sf::Vector2f offset = sf::Vector2f(0, 0));
    void drawNode(sf::RenderWindow& window, const Node& node,
        bool isSelected = false, float zoom = 1.0f,
        sf::Vector2f offset = sf::Vector2f(0, 0));
    void drawEdge(sf::RenderWindow& window, const Edge& edge,
        const Graph& graph, float zoom = 1.0f,
        sf::Vector2f offset = sf::Vector2f(0, 0));
    void drawRoute(sf::RenderWindow& window, const std::vector<int>& nodePath,
        const Graph& graph, float zoom = 1.0f,
        sf::Vector2f offset = sf::Vector2f(0, 0));

    sf::Color getTrafficColor(TrafficLevel level) const;
};