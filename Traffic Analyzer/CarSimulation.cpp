#include "CarSimulation.h"
#include "PredictionSystem.h"
#include <iostream>
#include <algorithm>
#include <queue>
#include <cmath>
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Car constructor
CarSimulation::Car::Car(int id, int start, int dest)
    : id(id), currentPosition(start), destination(dest),
    progress(0.0f), active(true) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(50, 255);
    color = sf::Color(dist(gen), dist(gen), dist(gen));
}

CarSimulation::CarSimulation(const Graph& map, PredictionSystem* predSystem)
    : cityMap(map), nextCarId(1), randomGen(std::random_device{}()),
    predictionSystem(predSystem), trafficSimulationActive(false),
    trafficSimulationTimer(0.0f), carSpawnInterval(2.0f),
    simulationSpeed(1.0f) {
}

void CarSimulation::toggleRunning() {
    trafficSimulationActive = !trafficSimulationActive;
    std::cout << "🚦 Traffic simulation "
        << (trafficSimulationActive ? "STARTED" : "STOPPED") << std::endl;

    if (trafficSimulationActive) {
        std::cout << "   - Automatic car spawning every " << carSpawnInterval << " seconds" << std::endl;
        std::cout << "   - Simulation speed: " << simulationSpeed << "x" << std::endl;
    }
}

void CarSimulation::spawnTrafficCar() {
    auto nodes = cityMap.getAllNodes();
    if (nodes.size() < 2) return;

    std::vector<int> nodeIds;
    for (const auto& pair : nodes) {
        nodeIds.push_back(pair.first);
    }

    std::uniform_int_distribution<> dist(0, static_cast<int>(nodeIds.size() - 1));
    int startNode = nodeIds[dist(randomGen)];
    int endNode = nodeIds[dist(randomGen)];

    int attempts = 0;
    while (endNode == startNode && nodeIds.size() > 1 && attempts < 10) {
        endNode = nodeIds[dist(randomGen)];
        attempts++;
    }

    if (startNode == endNode) return;

    auto route = calculateRoute(startNode, endNode);
    if (!route.empty()) {
        addCar(startNode, endNode, route);

        static int spawnCount = 0;
        if (++spawnCount % 5 == 0) {
            std::cout << "Traffic simulation: Added car #" << nextCarId - 1
                << " (Total: " << cars.size() << ")" << std::endl;
        }
    }
}

void CarSimulation::update(float deltaTime) {
    if (trafficSimulationActive) {
        trafficSimulationTimer += deltaTime * simulationSpeed;

        if (trafficSimulationTimer >= carSpawnInterval) {
            trafficSimulationTimer = 0.0f;

            int activeCars = getVehicleCount();

            if (activeCars < 100) {
                spawnTrafficCar();
            }
            else {
                std::cout << "Max car limit reached (" << activeCars << " cars)" << std::endl;
            }

            if (activeCars > 50) {
                carSpawnInterval = 5.0f / simulationSpeed;  
            }
            else if (activeCars > 20) {
                carSpawnInterval = 3.0f / simulationSpeed;  
            }
            else {
                carSpawnInterval = 2.0f / simulationSpeed;  
            }
        }
    }

    std::unordered_map<int, int> carsOnEdge;

    for (auto& car : cars) {
        if (!car.active || car.route.size() < 2) continue;

        auto it = std::find(car.route.begin(), car.route.end(), car.currentPosition);
        if (it == car.route.end() || it + 1 == car.route.end()) continue;

        int nextNode = *(it + 1);
        Edge edge = findEdge(car.currentPosition, nextNode);

        if (edge.id != -1) {
            carsOnEdge[edge.id]++;
        }
    }

    for (auto& car : cars) {
        if (!car.active || car.route.size() < 2) continue;

        auto it = std::find(car.route.begin(), car.route.end(), car.currentPosition);
        if (it == car.route.end() || it + 1 == car.route.end()) continue;

        int nextNode = *(it + 1);
        Edge edge = findEdge(car.currentPosition, nextNode);

        if (edge.id == -1) continue;

        int carsOnThisEdge = carsOnEdge[edge.id];
        float congestionFactor = 1.0f / (1.0f + carsOnThisEdge * 0.3f); 

        float baseSpeed = 1.0f;
        switch (edge.trafficLevel) {
        case TrafficLevel::FREE_FLOW: baseSpeed = 1.0f; break;
        case TrafficLevel::SLOW: baseSpeed = 0.6f; break;
        case TrafficLevel::CONGESTED: baseSpeed = 0.3f; break;
        case TrafficLevel::BLOCKED: baseSpeed = 0.0f; break;
        }

        float speed = baseSpeed * congestionFactor;

        car.progress += deltaTime * 0.5f * speed * simulationSpeed;

        if (car.progress >= 1.0f) {
            car.progress = 0.0f;
            car.currentPosition = nextNode;

            if (car.currentPosition == car.destination) {
                car.active = false;
                if (cars.size() < 20) {
                    std::cout << "Car " << car.id << " reached destination!" << std::endl;
                }
            }
        }
    }

    cars.erase(std::remove_if(cars.begin(), cars.end(),
        [](const Car& car) { return !car.active; }), cars.end());
}

void CarSimulation::addCar(int startNode, int endNode, const std::vector<int>& route) {
    if (route.size() < 2) return;

    Car newCar(nextCarId++, startNode, endNode);
    newCar.route = route;
    cars.push_back(newCar);

    if (nextCarId <= 10) {
        std::cout << "Car " << newCar.id << " added on route: ";
        for (int node : route) std::cout << node << " ";
        std::cout << std::endl;
    }
}

void CarSimulation::addRandomCar() {
    auto nodes = cityMap.getAllNodes();
    if (nodes.size() < 2) return;

    std::vector<int> nodeIds;
    for (const auto& pair : nodes) {
        nodeIds.push_back(pair.first);
    }

    std::uniform_int_distribution<> dist(0, static_cast<int>(nodeIds.size() - 1));
    int startNode = nodeIds[dist(randomGen)];
    int endNode = nodeIds[dist(randomGen)];

    while (endNode == startNode && nodeIds.size() > 1) {
        endNode = nodeIds[dist(randomGen)];
    }

    auto route = calculateRoute(startNode, endNode);
    if (!route.empty()) {
        addCar(startNode, endNode, route);
    }
}

void CarSimulation::draw(sf::RenderWindow& window, float zoom, sf::Vector2f offset) {
    for (const auto& car : cars) {
        if (!car.active || car.route.size() < 2) continue;

        auto it = std::find(car.route.begin(), car.route.end(), car.currentPosition);
        if (it == car.route.end() || it + 1 == car.route.end()) continue;

        int nextNode = *(it + 1);
        Node from = cityMap.getNode(car.currentPosition);
        Node to = cityMap.getNode(nextNode);

        if (from.id == -1 || to.id == -1) continue;

        float x = from.x + (to.x - from.x) * car.progress;
        float y = from.y + (to.y - from.y) * car.progress;

        float screenX = x * zoom + offset.x;
        float screenY = y * zoom + offset.y;

        sf::ConvexShape triangle(3);
        triangle.setPoint(0, sf::Vector2f(0, -8.0f * zoom));
        triangle.setPoint(1, sf::Vector2f(-5.0f * zoom, 5.0f * zoom));
        triangle.setPoint(2, sf::Vector2f(5.0f * zoom, 5.0f * zoom));

        triangle.setFillColor(car.color);
        triangle.setOutlineColor(sf::Color::White);
        triangle.setOutlineThickness(1.0f * zoom);
        triangle.setPosition(screenX, screenY);

        float dx = to.x - from.x;
        float dy = to.y - from.y;
        if (dx != 0 || dy != 0) {
            float angle = std::atan2(dy, dx) * 180.0f / M_PI;
            triangle.setRotation(angle);
        }

        window.draw(triangle);
    }
}

void CarSimulation::clearAllCars() {
    std::cout << "Clearing " << cars.size() << " cars" << std::endl;
    cars.clear();
    nextCarId = 1;
}

Edge CarSimulation::findEdge(int fromNode, int toNode) const {
    // Use optimized cache lookup instead of linear search
    return cityMap.findEdgeByNodes(fromNode, toNode);
}

// Calculate route using Dijkstra's algorithm
std::vector<int> CarSimulation::calculateRoute(int start, int end) {
    struct ComparePair {
        bool operator()(const std::pair<float, int>& a,
            const std::pair<float, int>& b) const {
            return a.first > b.first;
        }
    };

    std::priority_queue<std::pair<float, int>,
        std::vector<std::pair<float, int>>,
        ComparePair> pq;

    auto nodes = cityMap.getAllNodes();
    std::unordered_map<int, float> dist;
    std::unordered_map<int, int> prev;

    for (const auto& pair : nodes) {
        dist[pair.first] = std::numeric_limits<float>::max();
    }

    dist[start] = 0.0f;
    pq.push(std::make_pair(0.0f, start));

    while (!pq.empty()) {
        float currentDist = pq.top().first;
        int currentNode = pq.top().second;
        pq.pop();

        if (currentDist > dist[currentNode]) continue;
        if (currentNode == end) break;

        auto edges = cityMap.getEdgesFromNode(currentNode);
        for (int edgeId : edges) {
            Edge edge = cityMap.getEdge(edgeId);
            int neighbor = (edge.fromNodeId == currentNode) ? edge.toNodeId : edge.fromNodeId;

            float newDist = currentDist + edge.currentTravelTime;

            if (newDist < dist[neighbor]) {
                dist[neighbor] = newDist;
                prev[neighbor] = currentNode;
                pq.push(std::make_pair(newDist, neighbor));
            }
        }
    }

    if (dist.find(end) == dist.end() || dist[end] == std::numeric_limits<float>::max()) {
        return std::vector<int>();
    }

    std::vector<int> path;
    for (int at = end; at != start; at = prev[at]) {
        path.push_back(at);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());

    return path;
}

//void CarSimulation::rerouteIfNeeded(Vehicle& vehicle) {
//    // TODO: Implement if needed, or remove this method
//    std::cout << "Warning: CarSimulation::rerouteIfNeeded() not implemented" << std::endl;
//}