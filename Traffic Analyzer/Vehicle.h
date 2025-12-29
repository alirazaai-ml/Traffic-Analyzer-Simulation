#pragma once
#include "Graph.h"
#include <vector>
#include <SFML/Graphics.hpp>

struct PathResult {
    std::vector<int> nodePath;
    std::vector<int> edgePath;
    float totalTime;
    bool found;

    PathResult() : totalTime(0.0f), found(false) {}
};

class Vehicle {
private:
    int id;
    int currentNodeId;
    int targetNodeId;
    int currentEdgeId;
    float progressOnEdge; 
    std::vector<int> route;
    sf::CircleShape shape;
    float speed; // km/h
    bool isMoving;

public:
    Vehicle(int id, int startNode);

    void setRoute(const std::vector<int>& newRoute);
    void update(float deltaTime, const Graph& graph);
    void draw(sf::RenderWindow& window, const Graph& graph);

    int getId() const { return id; }
    int getCurrentNode() const { return currentNodeId; }
    bool hasReachedDestination() const;
    bool isOnRoute() const { return !route.empty(); }

private:
    void moveToNextNode(const Graph& graph);
};