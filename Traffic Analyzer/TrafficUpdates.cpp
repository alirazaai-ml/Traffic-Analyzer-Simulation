#include "TrafficUpdates.h"
#include <iostream>

TrafficUpdates::TrafficUpdates(Graph& g) 
    : graph(g), randomGenerator(std::chrono::system_clock::now().time_since_epoch().count()) {}

void TrafficUpdates::updateAllTraffic() {
    for (const auto& [edgeId, edge] : graph.getAllEdges()) {
        updateEdgeWithSensor(edgeId);
    }
}

void TrafficUpdates::updateEdgeWithSensor(int edgeId) {
    Edge edge = graph.getEdge(edgeId);
    
    std::normal_distribution<float> dist(edge.speedLimit * 0.8f, edge.speedLimit * 0.2f);
    float simulatedSpeed = dist(randomGenerator);
    
    simulatedSpeed = std::max(0.0f, std::min(simulatedSpeed, (float)edge.speedLimit));
    
    trafficSensors[edgeId].addReading(simulatedSpeed);
    float avgSpeed = trafficSensors[edgeId].getAverageSpeed();
    
    graph.updateEdgeTraffic(edgeId, avgSpeed);
    
    static int counter = 0;
    if (counter++ % 50 == 0) {
        std::cout << "Edge " << edgeId << " (" << edge.name 
                  << "): " << avgSpeed << " km/h" << std::endl;
    }
}

void TrafficUpdates::addRandomTrafficEvent() {
    if (graph.getAllEdges().empty()) return;
    
    auto edges = graph.getAllEdges();
    auto it = edges.begin();
    std::advance(it, randomGenerator() % edges.size());
    int randomEdge = it->first;
    
    trafficSensors[randomEdge].addReading(graph.getEdge(randomEdge).speedLimit * 0.2f);
    std::cout << "Traffic event on Edge " << randomEdge << std::endl;
}

void TrafficUpdates::simulatePeakHour() {
    std::cout << "Simulating peak hour traffic..." << std::endl;
    for (const auto& [edgeId, edge] : graph.getAllEdges()) {
        trafficSensors[edgeId].addReading(edge.speedLimit * 0.4f);
    }
    updateAllTraffic();
}

void TrafficUpdates::simulateAccident(int edgeId) {
    std::cout << "Accident on Edge " << edgeId << " - Road blocked!" << std::endl;
    trafficSensors[edgeId].addReading(0.0f);
    graph.updateEdgeTraffic(edgeId, 0.0f);
}

void TrafficUpdates::clearAccident(int edgeId) {
    std::cout << "Accident cleared on Edge " << edgeId << std::endl;
    Edge edge = graph.getEdge(edgeId);
    trafficSensors[edgeId].addReading(edge.speedLimit * 0.8f);
}