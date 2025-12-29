#include "Vehicle.h"
#include <iostream>
#include <algorithm>

Vehicle::Vehicle(int id, int startNode)
    : id(id), currentNodeId(startNode), targetNodeId(startNode),
    currentEdgeId(-1), progressOnEdge(0.0f), speed(40.0f), isMoving(false) {
    shape.setRadius(5.0f);
    shape.setFillColor(sf::Color::Red);
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(1.0f);
}

void Vehicle::setRoute(const std::vector<int>& newRoute) {
    route = newRoute;
    if (route.size() > 1) {
        currentNodeId = route[0];
        targetNodeId = route[1];
        isMoving = true;
        progressOnEdge = 0.0f;
    }
}

void Vehicle::update(float deltaTime, const Graph& graph) {
    if (!isMoving || route.size() < 2) return;

    if (currentEdgeId == -1) {
        auto edges = graph.getEdgesFromNode(currentNodeId);
        for (int edgeId : edges) {
            Edge edge = graph.getEdge(edgeId);
            if ((edge.fromNodeId == currentNodeId && edge.toNodeId == targetNodeId) ||
                (edge.fromNodeId == targetNodeId && edge.toNodeId == currentNodeId)) {
                currentEdgeId = edgeId;
                break;
            }
        }
    }

    if (currentEdgeId != -1) {
        Edge edge = graph.getEdge(currentEdgeId);

        float adjustedSpeed = speed;
        if (edge.trafficLevel == TrafficLevel::CONGESTED) {
            adjustedSpeed *= 0.3f;
        }
        else if (edge.trafficLevel == TrafficLevel::SLOW) {
            adjustedSpeed *= 0.6f;
        }

        float edgeTravelTime = edge.currentTravelTime; 
        float progressRate = 1.0f / (edgeTravelTime * 60.0f); 

        progressOnEdge += progressRate * deltaTime * (adjustedSpeed / 40.0f);

        if (progressOnEdge >= 1.0f) {
            currentNodeId = targetNodeId;

            auto it = std::find(route.begin(), route.end(), currentNodeId);
            if (it != route.end() && std::next(it) != route.end()) {
                targetNodeId = *std::next(it);
                currentEdgeId = -1;
                progressOnEdge = 0.0f;
            }
            else {
                isMoving = false;
            }
        }
    }
}

void Vehicle::draw(sf::RenderWindow& window, const Graph& graph) {
    if (!isMoving) return;

    Node fromNode = graph.getNode(currentNodeId);
    Node toNode = graph.getNode(targetNodeId);

    float x = fromNode.x + (toNode.x - fromNode.x) * progressOnEdge;
    float y = fromNode.y + (toNode.y - fromNode.y) * progressOnEdge;

    shape.setPosition(x - 5.0f, y - 5.0f);
    window.draw(shape);
}

bool Vehicle::hasReachedDestination() const {
    if (route.empty()) return true;
    return currentNodeId == route.back() && !isMoving;
}

void Vehicle::moveToNextNode(const Graph& graph) {
}