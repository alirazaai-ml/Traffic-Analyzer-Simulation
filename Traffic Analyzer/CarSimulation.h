#pragma once
#include "Graph.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
//#include "Vehicle.h"

class PredictionSystem;
struct TrafficPrediction;

class CarSimulation {
public:
    struct Car {
        int id;
        int currentPosition;
        int destination;
        float progress;
        bool active;
        sf::Color color;
        std::vector<int> route;

        Car(int id, int start, int dest);
    };

private:
    const Graph& cityMap;
    std::vector<Car> cars;
    int nextCarId;
    std::mt19937 randomGen;
    PredictionSystem* predictionSystem;

    bool trafficSimulationActive;
    float trafficSimulationTimer;
    float carSpawnInterval;  
    float simulationSpeed;

public:
    CarSimulation(const Graph& map, PredictionSystem* predSystem = nullptr);

    void addCar(int startNode, int endNode, const std::vector<int>& route);
    void addRandomCar();
    void update(float deltaTime);
    void draw(sf::RenderWindow& window, float zoom, sf::Vector2f offset);
    void clearAllCars();

    void toggleRunning();
    bool getIsRunning() const { return trafficSimulationActive; }
    void setSimulationSpeed(float speed) { simulationSpeed = speed; }

    int getVehicleCount() const { return static_cast<int>(cars.size()); }

private:
    Edge findEdge(int fromNode, int toNode) const;
    std::vector<int> calculateRoute(int start, int end);

    void spawnTrafficCar();
};