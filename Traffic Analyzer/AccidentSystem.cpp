#include "AccidentSystem.h"
#include "Graph.h"
#include <iostream>
#include <algorithm>

AccidentSystem::AccidentSystem(Graph* graph) : graphRef(graph) {}

void AccidentSystem::createAccident(int edgeId, float duration) {
    for (auto& accident : activeAccidents) {
        if (accident.edgeId == edgeId && accident.isActive) {
            std::cout << "Accident already active on edge " << edgeId << std::endl;
            return;
        }
    }

    Accident newAccident;
    newAccident.edgeId = edgeId;
    newAccident.duration = duration;
    newAccident.elapsed = 0.0f;
    newAccident.isActive = true;
    newAccident.clock.restart();

    activeAccidents.push_back(newAccident);

    if (graphRef) {
        graphRef->blockEdge(edgeId, duration);
    }

    std::cout << "Accident created on edge " << edgeId
        << " for " << duration << " seconds" << std::endl;
}

void AccidentSystem::clearAccident(int edgeId) {
    for (auto it = activeAccidents.begin(); it != activeAccidents.end(); ) {
        if (it->edgeId == edgeId) {
            it = activeAccidents.erase(it);
            std::cout << "Accident cleared from edge " << edgeId << std::endl;
        }
        else {
            ++it;
        }
    }
}

void AccidentSystem::clearAllAccidents() {
    activeAccidents.clear();
    std::cout << "All accidents cleared" << std::endl;
}

void AccidentSystem::update(float deltaTime) {
    for (auto it = activeAccidents.begin(); it != activeAccidents.end(); ) {
        if (it->isActive) {
            it->elapsed += deltaTime;

            if (it->elapsed >= it->duration) {
                std::cout << "Accident on edge " << it->edgeId << " has been cleared (time expired)" << std::endl;
                it = activeAccidents.erase(it);
                continue;
            }
        }
        ++it;
    }
}

bool AccidentSystem::hasAccidentOnEdge(int edgeId) const {
    for (const auto& accident : activeAccidents) {
        if (accident.edgeId == edgeId && accident.isActive) {
            return true;
        }
    }
    return false;
}

std::vector<int> AccidentSystem::getAccidentEdges() const {
    std::vector<int> result;
    for (const auto& accident : activeAccidents) {
        if (accident.isActive) {
            result.push_back(accident.edgeId);
        }
    }
    return result;
}

int AccidentSystem::getActiveAccidentCount() const {
    int count = 0;
    for (const auto& accident : activeAccidents) {
        if (accident.isActive) count++;
    }
    return count;
}

sf::Color AccidentSystem::getEdgeColorWithAccident(int edgeId, sf::Color originalColor) const {
    if (!hasAccidentOnEdge(edgeId)) {
        return originalColor;
    }

    float time = 0.0f;
    for (const auto& accident : activeAccidents) {
        if (accident.edgeId == edgeId) {
            time = accident.clock.getElapsedTime().asSeconds();
            break;
        }
    }

    bool isVisible = static_cast<int>(time * 2.0f) % 2 == 0;

    if (isVisible) {
        return sf::Color(255, 50, 50); // Bright red
    }
    else {
        return sf::Color(150, 0, 0);   // Dark red
    }
}

bool AccidentSystem::shouldBlink(int edgeId) const {
    if (!hasAccidentOnEdge(edgeId)) return false;

    float time = 0.0f;
    for (const auto& accident : activeAccidents) {
        if (accident.edgeId == edgeId) {
            time = accident.clock.getElapsedTime().asSeconds();
            break;
        }
    }

    return static_cast<int>(time * 2.0f) % 2 == 0;
}

void AccidentSystem::createRandomAccident() {
    if (!graphRef) {
        std::cout << "Error: No graph reference for creating accident" << std::endl;
        return;
    }

    auto edges = graphRef->getAllEdges();
    if (edges.empty()) {
        std::cout << "Error: No edges available for accident creation" << std::endl;
        return;
    }

    std::vector<int> edgeIds;
    for (const auto& pair : edges) {
        if (!hasAccidentOnEdge(pair.first)) {
            edgeIds.push_back(pair.first);
        }
    }

    if (edgeIds.empty()) {
        std::cout << "All edges already have accidents!" << std::endl;
        return;
    }

    int randomIndex = rand() % edgeIds.size();
    int selectedEdgeId = edgeIds[randomIndex];

    float duration = 60.0f + (rand() % 300);

    createAccident(selectedEdgeId, duration);
}