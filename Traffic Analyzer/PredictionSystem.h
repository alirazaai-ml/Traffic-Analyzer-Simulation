// PredictionSystem.h - FIXED VERSION
#pragma once
#include <vector>
#include <deque>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "Graph.h"

struct TrafficPrediction {
    int edgeId;
    float currentSpeed;
    float predictedSpeed5min;
    float predictedSpeed10min;
    float confidence; // 0.0 to 1.0
    bool willBeCongested;

    TrafficPrediction()
        : edgeId(-1), currentSpeed(0), predictedSpeed5min(0),
        predictedSpeed10min(0), confidence(0), willBeCongested(false) {
    }

    TrafficPrediction(int id, float current, float pred5, float pred10, float conf)
        : edgeId(id), currentSpeed(current), predictedSpeed5min(pred5),
        predictedSpeed10min(pred10), confidence(conf),
        willBeCongested(pred5 < 20.0f) {
    }
};

class PredictionSystem {
private:
    Graph* graph;

    // Store historical data for each edge
    struct EdgeHistory {
        std::deque<float> speeds;     // Last hour of speed data (60 samples if 1/min)
        std::deque<float> predictions; // Previous predictions for accuracy calculation
    };

    std::unordered_map<int, EdgeHistory> edgeHistories;

    // Configuration
    const int MAX_HISTORY_SIZE = 2;   // 2 minutes of data
    const float PREDICTION_INTERVAL = 5.0f; // Predict every 5 seconds

    // Performance tracking
    float predictionTimer;

public:
    PredictionSystem(Graph* graph);

    // Main methods
    void update(float deltaTime);
    TrafficPrediction predictEdge(int edgeId);
    TrafficPrediction predictEdgeConst(int edgeId) const;  // ADDED const version
    std::vector<TrafficPrediction> predictAllEdges();

    // Advanced predictions
    std::vector<int> getEdgesLikelyToCongest(int minutesAhead = 5);
    float getRoutePredictedTime(const std::vector<int>& path, int minutesAhead = 5);

    // Statistics
    float getAveragePredictionAccuracy() const;
    int getPredictedCongestionCount();

private:
    // Prediction algorithms - FIXED: remove duplicate declarations
    float simpleMovingAverage(const std::deque<float>& data, int window = 10) const;
    float weightedMovingAverage(const std::deque<float>& data) const;
    float exponentialSmoothing(const std::deque<float>& data, float alpha = 0.3f) const;
    float linearRegressionPrediction(const std::deque<float>& data) const;
    float calculateConfidence(const std::deque<float>& history) const;  // ONLY ONE DECLARATION
    bool isPeakHour() const;  // ONLY ONE DECLARATION

    // Helper methods
    void addSpeedData(int edgeId, float speed);

    // Internal prediction method
    TrafficPrediction predictEdgeInternal(int edgeId, bool updateHistory = false) const;
};