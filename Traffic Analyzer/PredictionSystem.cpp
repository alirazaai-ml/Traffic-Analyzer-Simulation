#include "PredictionSystem.h"

PredictionSystem::PredictionSystem(Graph* graph)
    : graph(graph), predictionTimer(0.0f) {
}

// ================== MISSING METHOD ADDED ==================
void PredictionSystem::addSpeedData(int edgeId, float speed) {
    auto& history = edgeHistories[edgeId];
    history.speeds.push_back(speed);

    if (history.speeds.size() > MAX_HISTORY_SIZE) {
        history.speeds.pop_front();
    }
}

void PredictionSystem::update(float deltaTime) {
    predictionTimer += deltaTime;

    if (predictionTimer >= PREDICTION_INTERVAL) {
        predictionTimer = 0.0f;

        auto edges = graph->getAllEdges();
        for (const auto& pair : edges) {
            const Edge& edge = pair.second;

            // Calculate current speed from travel time
            float currentSpeed = (edge.length / edge.currentTravelTime) * 60.0f;

            addSpeedData(edge.id, currentSpeed);  // NOW THIS WILL WORK
        }
    }
}

TrafficPrediction PredictionSystem::predictEdgeInternal(int edgeId, bool updateHistory) const {
    TrafficPrediction prediction;
    prediction.edgeId = edgeId;

    auto it = edgeHistories.find(edgeId);
    if (it == edgeHistories.end() || it->second.speeds.empty()) {
        Edge edge = graph->getEdge(edgeId);
        prediction.currentSpeed = edge.speedLimit;
        prediction.predictedSpeed5min = edge.speedLimit;
        prediction.predictedSpeed10min = edge.speedLimit;
        prediction.confidence = 0.0f;
        prediction.willBeCongested = false;
        return prediction;
    }

    const auto& speeds = it->second.speeds;
    float currentSpeed = speeds.back();
    prediction.currentSpeed = currentSpeed;

    // Use multiple algorithms
    float pred1 = exponentialSmoothing(speeds);
    float pred2 = weightedMovingAverage(speeds);
    float pred3 = simpleMovingAverage(speeds);

    prediction.predictedSpeed5min = (pred1 + pred2 + pred3) / 3.0f;

    float trend = prediction.predictedSpeed5min - currentSpeed;
    prediction.predictedSpeed10min = prediction.predictedSpeed5min + (trend * 0.5f);

    if (isPeakHour()) {
        prediction.predictedSpeed5min *= 0.7f;
        prediction.predictedSpeed10min *= 0.6f;
    }

    Edge edge = graph->getEdge(edgeId);
    prediction.predictedSpeed5min = std::max(5.0f,
        std::min(prediction.predictedSpeed5min, (float)edge.speedLimit));
    prediction.predictedSpeed10min = std::max(5.0f,
        std::min(prediction.predictedSpeed10min, (float)edge.speedLimit));

    prediction.confidence = calculateConfidence(speeds);
    prediction.willBeCongested = prediction.predictedSpeed5min < 20.0f;

    return prediction;
}

TrafficPrediction PredictionSystem::predictEdge(int edgeId) {
    TrafficPrediction prediction = predictEdgeInternal(edgeId, false);

    // Update history for non-const version only
    edgeHistories[edgeId].predictions.push_back(prediction.predictedSpeed5min);
    if (edgeHistories[edgeId].predictions.size() > 20) {
        edgeHistories[edgeId].predictions.pop_front();
    }

    return prediction;
}

TrafficPrediction PredictionSystem::predictEdgeConst(int edgeId) const {
    return predictEdgeInternal(edgeId, false);
}

std::vector<TrafficPrediction> PredictionSystem::predictAllEdges() {
    std::vector<TrafficPrediction> predictions;
    auto edges = graph->getAllEdges();

    predictions.reserve(edges.size());
    for (const auto& pair : edges) {
        predictions.push_back(predictEdge(pair.first));
    }

    return predictions;
}

std::vector<int> PredictionSystem::getEdgesLikelyToCongest(int minutesAhead) {
    std::vector<std::pair<int, float>> edgeCongestion;

    auto predictions = predictAllEdges();
    for (const auto& pred : predictions) {
        float predictedSpeed = (minutesAhead <= 5) ?
            pred.predictedSpeed5min : pred.predictedSpeed10min;

        Edge edge = graph->getEdge(pred.edgeId);
        float congestionProb = 1.0f - (predictedSpeed / edge.speedLimit);

        if (congestionProb > 0.5f && pred.confidence > 0.6f) {
            edgeCongestion.push_back({ pred.edgeId, congestionProb });
        }
    }

    std::sort(edgeCongestion.begin(), edgeCongestion.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

    std::vector<int> result;
    result.reserve(edgeCongestion.size());
    for (const auto& pair : edgeCongestion) {
        result.push_back(pair.first);
    }

    return result;
}

float PredictionSystem::getRoutePredictedTime(const std::vector<int>& path, int minutesAhead) {
    if (path.size() < 2) return 0.0f;

    float totalTime = 0.0f;

    for (size_t i = 0; i < path.size() - 1; i++) {
        int fromNode = path[i];
        int toNode = path[i + 1];

        auto edges = graph->getAllEdges();
        int edgeId = -1;

        for (const auto& pair : edges) {
            if (pair.second.fromNodeId == fromNode && pair.second.toNodeId == toNode) {
                edgeId = pair.first;
                break;
            }
        }

        if (edgeId != -1) {
            TrafficPrediction pred = predictEdge(edgeId);
            float predictedSpeed = (minutesAhead <= 5) ?
                pred.predictedSpeed5min : pred.predictedSpeed10min;

            Edge edge = graph->getEdge(edgeId);
            float predictedTravelTime = (edge.length / predictedSpeed) * 60.0f;

            totalTime += predictedTravelTime;
        }
    }

    return totalTime;
}

// ================== PREDICTION ALGORITHMS ==================

float PredictionSystem::simpleMovingAverage(const std::deque<float>& data, int window) const {
    if (data.empty()) return 0.0f;

    int actualWindow = std::min(window, (int)data.size());
    float sum = 0.0f;

    for (int i = data.size() - actualWindow; i < data.size(); i++) {
        sum += data[i];
    }

    return sum / actualWindow;
}

float PredictionSystem::weightedMovingAverage(const std::deque<float>& data) const {
    if (data.empty()) return 0.0f;

    float sum = 0.0f;
    float weightSum = 0.0f;

    for (size_t i = 0; i < data.size(); i++) {
        float weight = (i + 1.0f) / data.size();
        sum += data[i] * weight;
        weightSum += weight;
    }

    return sum / weightSum;
}

float PredictionSystem::exponentialSmoothing(const std::deque<float>& data, float alpha) const {
    if (data.empty()) return 0.0f;

    float result = data[0];
    for (size_t i = 1; i < data.size(); i++) {
        result = alpha * data[i] + (1 - alpha) * result;
    }

    return result;
}

float PredictionSystem::linearRegressionPrediction(const std::deque<float>& data) const {
    if (data.size() < 3) return data.empty() ? 0.0f : data.back();

    float sumX = 0.0f, sumY = 0.0f, sumXY = 0.0f, sumX2 = 0.0f;
    int n = data.size();

    for (int i = 0; i < n; i++) {
        float x = i;
        float y = data[i];
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    float m = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    float b = (sumY - m * sumX) / n;

    return m * n + b;
}

float PredictionSystem::calculateConfidence(const std::deque<float>& history) const {
    if (history.size() < 10) return 0.3f;

    float mean = 0.0f;
    for (float speed : history) {
        mean += speed;
    }
    mean /= history.size();

    float variance = 0.0f;
    for (float speed : history) {
        variance += (speed - mean) * (speed - mean);
    }
    variance /= history.size();

    float stdDev = sqrt(variance);
    float coefficientOfVariation = stdDev / mean;

    float confidence = 1.0f - std::min(coefficientOfVariation, 1.0f);
    float dataConfidence = std::min(history.size() / 30.0f, 1.0f);

    return (confidence * 0.7f + dataConfidence * 0.3f);
}

bool PredictionSystem::isPeakHour() const {
    // Simple simulation - returns true 50% of the time
    static int counter = 0;
    counter++;
    return (counter / 60) % 2 == 0; // Toggle every 60 calls
}

float PredictionSystem::getAveragePredictionAccuracy() const {
    if (edgeHistories.empty()) return 0.0f;

    float totalAccuracy = 0.0f;
    int count = 0;

    for (const auto& pair : edgeHistories) {
        if (pair.second.predictions.size() >= 2) {
            const auto& preds = pair.second.predictions;
            const auto& speeds = pair.second.speeds;

            if (preds.size() >= 2 && speeds.size() >= preds.size() + 1) {
                float errorSum = 0.0f;
                for (size_t i = 0; i < preds.size() - 1; i++) {
                    float actual = speeds[i + 1];
                    float predicted = preds[i];
                    float error = fabs(actual - predicted) / actual;
                    errorSum += std::min(error, 1.0f);
                }
                totalAccuracy += 1.0f - (errorSum / (preds.size() - 1));
                count++;
            }
        }
    }

    return count > 0 ? totalAccuracy / count : 0.0f;
}

int PredictionSystem::getPredictedCongestionCount() {
    int count = 0;
    auto predictions = predictAllEdges();

    for (const auto& pred : predictions) {
        if (pred.willBeCongested && pred.confidence > 0.6f) {
            count++;
        }
    }

    return count;
}