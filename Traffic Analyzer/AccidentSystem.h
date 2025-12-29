// AccidentSystem.h
#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

class Graph; // Forward declaration

struct Accident {
    int edgeId;
    float duration;      // Total duration in seconds
    float elapsed;       // Time elapsed in seconds
    bool isActive;
    sf::Clock clock;     // For visual effects
};

class AccidentSystem {
private:
    std::vector<Accident> activeAccidents;
    Graph* graphRef;

public:
    AccidentSystem(Graph* graph);

    // Core functionality
    void createAccident(int edgeId, float duration = 300.0f); // 5 minutes default
    void clearAccident(int edgeId);
    void clearAllAccidents();
    void update(float deltaTime);

    // Query methods
    bool hasAccidentOnEdge(int edgeId) const;
    std::vector<int> getAccidentEdges() const;
    int getActiveAccidentCount() const;

    // Visual effects
    sf::Color getEdgeColorWithAccident(int edgeId, sf::Color originalColor) const;
    bool shouldBlink(int edgeId) const; // For blinking effect
};