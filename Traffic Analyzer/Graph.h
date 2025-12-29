#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <limits>
#include <algorithm>

enum class TrafficLevel {
    FREE_FLOW = 0,
    SLOW = 1,
    CONGESTED = 2,
    BLOCKED = 3
};

struct Node {
    int id;
    float x, y;
    std::string name;

    Node(int id = -1, float x = 0, float y = 0, std::string name = "")
        : id(id), x(x), y(y), name(name) {
    }
};

struct Edge {
    int id;
    int fromNodeId;
    int toNodeId;
    float length;
    int speedLimit;
    float baseTravelTime;
    float currentTravelTime;
    TrafficLevel trafficLevel;
    std::string name;
    float distance; 

    bool isBlocked;  
    float accidentTimer; 

    Edge(int id = -1, int from = -1, int to = -1, float len = 1.0f,
        int limit = 60, std::string name = "")
        : id(id), fromNodeId(from), toNodeId(to), length(len), speedLimit(limit),
        name(name), distance(len), isBlocked(false), accidentTimer(0.0f)  
    {
        baseTravelTime = (length / (float)speedLimit) * 60.0f;
        currentTravelTime = baseTravelTime;
        trafficLevel = TrafficLevel::FREE_FLOW;
    }

    void updateTraffic(float currentSpeed);

    void setBlocked(bool blocked, float duration = 0.0f) {
        isBlocked = blocked;
        if (blocked) {
            accidentTimer = duration;
            trafficLevel = TrafficLevel::BLOCKED;
            currentTravelTime = baseTravelTime * 10.0f; 
        }
        else {
            accidentTimer = 0.0f;
            trafficLevel = TrafficLevel::FREE_FLOW;
            currentTravelTime = baseTravelTime;
        }
    }

    void updateAccidentTimer(float deltaTime) {
        if (isBlocked && accidentTimer > 0) {
            accidentTimer -= deltaTime;
            if (accidentTimer <= 0) {
                setBlocked(false);
            }
        }
    }

    bool hasAccident() const { return isBlocked; }
};

class Graph {
private:
    std::unordered_map<int, Node> nodes;
    std::unordered_map<int, Edge> edges;
    std::unordered_map<int, std::vector<int>> adjacencyList;

public:
    Graph() = default;

    void addNode(int id, float x, float y, const std::string& name = "");
    Node getNode(int id) const;
    bool hasNode(int id) const;
    const std::unordered_map<int, Node>& getAllNodes() const;

    void addEdge(int id, int from, int to, float length,
        int speedLimit = 60, const std::string& name = "");
    Edge getEdge(int id) const;
    bool hasEdge(int id) const;
    const std::unordered_map<int, Edge>& getAllEdges() const;

    std::vector<int> getEdgesFromNode(int nodeId) const;
    std::vector<int> findShortestPath(int start, int end) const;
    int getNodeCount() const;
    int getEdgeCount() const;

    void updateEdgeTraffic(int edgeId, float currentSpeed);
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

    void blockEdge(int edgeId, float duration = 300.0f) {
        if (edges.find(edgeId) != edges.end()) {
            edges[edgeId].setBlocked(true, duration);
            std::cout << "Edge " << edgeId << " blocked for " << duration << " seconds" << std::endl;
        }
    }

    void unblockEdge(int edgeId) {
        if (edges.find(edgeId) != edges.end()) {
            edges[edgeId].setBlocked(false);
            std::cout << "Edge " << edgeId << " unblocked" << std::endl;
        }
    }

    bool isEdgeBlocked(int edgeId) const {
        auto it = edges.find(edgeId);
        return it != edges.end() && it->second.hasAccident();
    }

    void updateAccidents(float deltaTime) {
        for (auto& pair : edges) {
            pair.second.updateAccidentTimer(deltaTime);
        }
    }

    void clearAll() {
        nodes.clear();
        edges.clear();
        adjacencyList.clear();
        std::cout << "[Graph] Completely cleared all data" << std::endl;
    }

    void removeAllNodes() {
        int count = nodes.size();
        nodes.clear();
        adjacencyList.clear(); 
        std::cout << "[Graph] Removed " << count << " nodes" << std::endl;
    }

    void removeAllEdges() {
        int count = edges.size();
        edges.clear();
        for (auto& pair : adjacencyList) {
            pair.second.clear();
        }
        std::cout << "[Graph] Removed " << count << " edges" << std::endl;
    }
};