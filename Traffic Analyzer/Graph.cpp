#include "Graph.h"

void Edge::updateTraffic(float currentSpeed) {
    if (currentSpeed <= 0) {
        trafficLevel = TrafficLevel::BLOCKED;
        currentTravelTime = 9999.0f;
    }
    else if (currentSpeed < speedLimit * 0.3f) {
        trafficLevel = TrafficLevel::CONGESTED;
        currentTravelTime = baseTravelTime * 3.0f;
    }
    else if (currentSpeed < speedLimit * 0.7f) {
        trafficLevel = TrafficLevel::SLOW;
        currentTravelTime = baseTravelTime * 1.5f;
    }
    else {
        trafficLevel = TrafficLevel::FREE_FLOW;
        currentTravelTime = baseTravelTime;
    }
}

void Graph::addNode(int id, float x, float y, const std::string& name) {
    nodes[id] = Node(id, x, y, name);
    adjacencyList[id] = std::vector<int>();
}

void Graph::addEdge(int id, int from, int to, float length,
    int speedLimit, const std::string& name) {
    edges[id] = Edge(id, from, to, length, speedLimit, name);
    adjacencyList[from].push_back(id);
    adjacencyList[to].push_back(id);
    
    // Add to cache for fast lookup
    edgeCache.addEdge(from, to, id);
}

Node Graph::getNode(int id) const {
    auto it = nodes.find(id);
    if (it != nodes.end()) return it->second;
    return Node(-1, 0, 0, "");
}

bool Graph::hasNode(int id) const {
    return nodes.find(id) != nodes.end();
}

Edge Graph::getEdge(int id) const {
    auto it = edges.find(id);
    if (it != edges.end()) return it->second;
    return Edge(-1, -1, -1, 0.0f, 0, "");
}

bool Graph::hasEdge(int id) const {
    return edges.find(id) != edges.end();
}

// Optimized edge lookup using cache
int Graph::findEdgeId(int fromNode, int toNode) const {
    return edgeCache.findEdge(fromNode, toNode);
}

Edge Graph::findEdgeByNodes(int fromNode, int toNode) const {
    int edgeId = findEdgeId(fromNode, toNode);
    if (edgeId != -1) {
        return getEdge(edgeId);
    }
    return Edge(-1, -1, -1, 0.0f, 0, "");
}

void Graph::rebuildEdgeCache() {
    edgeCache.clear();
    for (const auto& pair : edges) {
        const Edge& edge = pair.second;
        edgeCache.addEdge(edge.fromNodeId, edge.toNodeId, edge.id);
    }
    edgeCache.markClean();
}

void Graph::clearGraph() {
    nodes.clear();
    edges.clear();
    adjacencyList.clear();
    edgeCache.clear();
}

const std::unordered_map<int, Node>& Graph::getAllNodes() const {
    return nodes;
}

const std::unordered_map<int, Edge>& Graph::getAllEdges() const {
    return edges;
}

std::vector<int> Graph::getEdgesFromNode(int nodeId) const {
    auto it = adjacencyList.find(nodeId);
    if (it != adjacencyList.end()) return it->second;
    return std::vector<int>();
}

int Graph::getNodeCount() const {
    return static_cast<int>(nodes.size());
}

int Graph::getEdgeCount() const {
    return static_cast<int>(edges.size());
}

void Graph::updateEdgeTraffic(int edgeId, float currentSpeed) {
    auto it = edges.find(edgeId);
    if (it != edges.end()) {
        it->second.updateTraffic(currentSpeed);
    }
}

void Graph::blockEdge(int edgeId, float duration) {
    auto it = edges.find(edgeId);
    if (it != edges.end()) {
        it->second.setBlocked(true, duration);
    }
}

void Graph::unblockEdge(int edgeId) {
    auto it = edges.find(edgeId);
    if (it != edges.end()) {
        it->second.setBlocked(false);
    }
}

bool Graph::isEdgeBlocked(int edgeId) const {
    auto it = edges.find(edgeId);
    if (it != edges.end()) {
        return it->second.isBlocked;
    }
    return false;
}

void Graph::updateAccidents(float deltaTime) {
    for (auto& pair : edges) {
        pair.second.updateAccidentTimer(deltaTime);
    }
}

std::vector<int> Graph::findShortestPath(int start, int end) const {
    if (!hasNode(start) || !hasNode(end)) {
        return std::vector<int>();
    }

    // Custom comparator for priority queue
    struct ComparePair {
        bool operator()(const std::pair<float, int>& a,
            const std::pair<float, int>& b) const {
            return a.first > b.first; // Min-heap
        }
    };

    // Priority queue for Dijkstra: (distance, node)
    std::priority_queue<std::pair<float, int>,
        std::vector<std::pair<float, int>>,
        ComparePair> pq;

    std::unordered_map<int, float> dist;
    std::unordered_map<int, int> prev;

    // Initialize distances
    for (const auto& pair : nodes) {
        dist[pair.first] = std::numeric_limits<float>::max();
    }

    dist[start] = 0.0f;
    pq.push(std::make_pair(0.0f, start));

    while (!pq.empty()) {
        float currentDist = pq.top().first;
        int currentNode = pq.top().second;
        pq.pop();

        if (currentDist > dist[currentNode]) {
            continue;
        }

        if (currentNode == end) {
            break;
        }

        // Explore neighbors
        auto edgesFromNode = getEdgesFromNode(currentNode);
        for (int edgeId : edgesFromNode) {
            Edge edge = getEdge(edgeId);
            int neighbor = (edge.fromNodeId == currentNode) ? edge.toNodeId : edge.fromNodeId;

            float newDist = currentDist + edge.currentTravelTime;

            if (newDist < dist[neighbor]) {
                dist[neighbor] = newDist;
                prev[neighbor] = currentNode;
                pq.push(std::make_pair(newDist, neighbor));
            }
        }
    }

    if (dist[end] == std::numeric_limits<float>::max()) {
        return std::vector<int>();
    }

    // Reconstruct path
    std::vector<int> path;
    for (int at = end; at != start; at = prev[at]) {
        path.push_back(at);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());

    return path;
}

void Graph::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    file << "[Nodes]" << std::endl;
    for (const auto& pair : nodes) {
        const Node& node = pair.second;
        file << node.id << "," << node.x << "," << node.y << "," << node.name << std::endl;
    }

    file << "\n[Edges]" << std::endl;
    for (const auto& pair : edges) {
        const Edge& edge = pair.second;
        file << edge.id << "," << edge.fromNodeId << "," << edge.toNodeId << ","
            << edge.length << "," << edge.speedLimit << "," << edge.name << std::endl;
    }

    file.close();
}

void Graph::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    nodes.clear();
    edges.clear();
    adjacencyList.clear();

    std::string line;
    std::string section = "";

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line == "[Nodes]") {
            section = "Nodes";
            continue;
        }
        else if (line == "[Edges]") {
            section = "Edges";
            continue;
        }

        std::stringstream ss(line);
        std::string token;

        if (section == "Nodes") {
            std::vector<std::string> tokens;
            while (std::getline(ss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 3) {
                int id = std::stoi(tokens[0]);
                float x = std::stof(tokens[1]);
                float y = std::stof(tokens[2]);
                std::string name = (tokens.size() > 3) ? tokens[3] : "";
                addNode(id, x, y, name);
            }
        }
        else if (section == "Edges") {
            std::vector<std::string> tokens;
            while (std::getline(ss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 5) {
                int id = std::stoi(tokens[0]);
                int from = std::stoi(tokens[1]);
                int to = std::stoi(tokens[2]);
                float length = std::stof(tokens[3]);
                int speedLimit = std::stoi(tokens[4]);
                std::string name = (tokens.size() > 5) ? tokens[5] : "";
                addEdge(id, from, to, length, speedLimit, name);
            }
        }
    }

    file.close();
    rebuildEdgeCache();
}
