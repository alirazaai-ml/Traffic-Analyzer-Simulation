#include "MapGenerator.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <ctime>  
#include "Graph.h"

MapGenerator::CityType MapGenerator::currentCityType = CityType::SIMPLE_GRID;

Graph MapGenerator::generateCity() {
    std::cout << "\n=== GENERATING NEW CITY ===" << std::endl;
    
    Graph newGraph;
    
    generateNextCity(newGraph);
    
    std::cout << "City generation complete!" << std::endl;
    std::cout << "  Nodes: " << newGraph.getNodeCount() << std::endl;
    std::cout << "  Edges: " << newGraph.getEdgeCount() << std::endl;
    
    return newGraph;
}

int MapGenerator::getNextNodeId(const Graph& graph) {
    auto nodes = graph.getAllNodes();

    if (nodes.empty()) {
        return 1;
    }

    int maxId = 0;
    for (const auto& pair : nodes) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }

    return maxId + 1;
}

int MapGenerator::getNextEdgeId(const Graph& graph) {
    auto edges = graph.getAllEdges();

    if (edges.empty()) {
        return 1;
    }

    int maxId = 0;
    for (const auto& pair : edges) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }

    return maxId + 1;
}

void MapGenerator::addTrafficHotspot(Graph& graph, int centerNode, float congestionLevel) {
    std::cout << "Creating traffic hotspot at node " << centerNode
        << " with congestion " << (congestionLevel * 100) << "%" << std::endl;

    auto edges = graph.getEdgesFromNode(centerNode);

    if (edges.empty()) {
        std::cout << "Warning: Node " << centerNode << " has no connected roads!" << std::endl;
        return;
    }

    std::cout << "Found " << edges.size() << " roads connected to node " << centerNode << std::endl;

    for (int edgeId : edges) {
        Edge edge = graph.getEdge(edgeId);

        if (edge.id == -1) {
            std::cout << "Warning: Edge " << edgeId << " not found!" << std::endl;
            continue;
        }

        float slowedSpeed = edge.speedLimit * (1.0f - congestionLevel);

        if (slowedSpeed < 5.0f) {
            slowedSpeed = 5.0f;
        }

        graph.updateEdgeTraffic(edgeId, slowedSpeed);

        std::cout << "  Edge " << edgeId << " (" << edge.name
            << ") speed reduced from " << edge.speedLimit
            << " to " << slowedSpeed << " km/h" << std::endl;
    }

    std::cout << "Traffic hotspot created successfully!" << std::endl;
}

void MapGenerator::generateSimpleGrid(Graph& graph, int gridSize) {
    std::cout << "Generating " << gridSize << "x" << gridSize << " grid city..." << std::endl;

    float spacing = 80.0f;
    float startX = 100.0f;
    float startY = 100.0f;

    int nodeId = 1;
    for (int row = 0; row < gridSize; row++) {
        for (int col = 0; col < gridSize; col++) {
            float x = startX + col * spacing;
            float y = startY + row * spacing;

            std::string name = "N" + std::to_string(row) + "-" + std::to_string(col);
            graph.addNode(nodeId, x, y, name);
            nodeId++;
        }
    }

    int edgeId = 1;
    for (int row = 0; row < gridSize; row++) {
        for (int col = 0; col < gridSize - 1; col++) {
            int from = row * gridSize + col + 1;
            int to = row * gridSize + col + 2;

            int speedLimit = 50; 
            if (row == 0 || row == gridSize - 1) speedLimit = 70; 

            graph.addEdge(edgeId++, from, to, spacing, speedLimit,
                "H" + std::to_string(row) + "-" + std::to_string(col));
        }
    }

    for (int col = 0; col < gridSize; col++) {
        for (int row = 0; row < gridSize - 1; row++) {
            int from = row * gridSize + col + 1;
            int to = (row + 1) * gridSize + col + 1;

            int speedLimit = 50; 
            if (col == 0 || col == gridSize - 1) speedLimit = 70; 

            graph.addEdge(edgeId++, from, to, spacing, speedLimit,
                "V" + std::to_string(row) + "-" + std::to_string(col));
        }
    }

    std::cout << "Generated " << gridSize * gridSize << " nodes and "
        << (gridSize * (gridSize - 1) * 2) << " edges." << std::endl;
}

void MapGenerator::generateComplexCity(Graph& graph) {
    std::cout << "\n  CREATING COMPLEX CITY..." << std::endl;



    static int cityLayout = 0;
    cityLayout = (cityLayout + 1) % 3;

    if (cityLayout == 0) {
        std::cout << "Generating GRID CITY layout..." << std::endl;
        generateGridCity(graph);
    }
    else if (cityLayout == 1) {
        std::cout << "Generating RADIAL CITY layout..." << std::endl;
        generateRadialCity(graph);
    }
    else {
        std::cout << "Generating ORGANIC CITY layout..." << std::endl;
        generateOrganicCity(graph);
    }

    std::cout << " Complex city generated!" << std::endl;
}

void MapGenerator::generateGridCity(Graph& graph) {
    std::cout << "Generating GRID CITY layout..." << std::endl;

    float spacing = 80.0f;
    int nodeId = 1;
    int edgeId = 1;

    int gridNodes[4][4];
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float x = 150.0f + col * spacing;
            float y = 150.0f + row * spacing;
            graph.addNode(nodeId, x, y, "G" + std::to_string(nodeId));
            gridNodes[row][col] = nodeId;
            nodeId++;
        }
    }

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            graph.addEdge(edgeId++, gridNodes[row][col], gridNodes[row][col + 1],
                spacing, 50, "Street");
        }
    }

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 3; row++) {
            graph.addEdge(edgeId++, gridNodes[row][col], gridNodes[row + 1][col],
                spacing, 50, "Avenue");
        }
    }

    graph.addEdge(edgeId++, gridNodes[0][0], gridNodes[1][1], spacing * 1.414f, 40, "Diagonal");
    graph.addEdge(edgeId++, gridNodes[2][2], gridNodes[3][3], spacing * 1.414f, 40, "Diagonal");
}

void MapGenerator::createRoundabout(Graph& graph, float centerX, float centerY, float radius) {
    std::cout << "Creating roundabout at (" << centerX << ", " << centerY
        << ") with radius " << radius << std::endl;

    int baseNodeId = getNextNodeId(graph);
    int baseEdgeId = getNextEdgeId(graph);

    std::vector<int> roundaboutNodes;

    for (int i = 0; i < 4; i++) {
        float angle = i * 90.0f * 3.14159f / 180.0f;
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);

        int nodeId = baseNodeId++;
        graph.addNode(nodeId, x, y, "Roundabout-" + std::to_string(i));
        roundaboutNodes.push_back(nodeId);
    }

    for (int i = 0; i < 4; i++) {
        int from = roundaboutNodes[i];
        int to = roundaboutNodes[(i + 1) % 4];

        float arcLength = radius * 3.14159f / 2.0f;

        graph.addEdge(baseEdgeId++, from, to, arcLength, 30,
            "Roundabout-" + std::to_string(i));
    }

    std::cout << "Roundabout created with 4 nodes and 4 edges" << std::endl;
}

void MapGenerator::addBridge(Graph& graph, int node1, int node2, bool isOverpass) {
    std::cout << "Adding " << (isOverpass ? "overpass" : "bridge")
        << " between nodes " << node1 << " and " << node2 << std::endl;

    Node n1 = graph.getNode(node1);
    Node n2 = graph.getNode(node2);

    if (n1.id == -1 || n2.id == -1) {
        std::cout << "Error: One or both nodes not found!" << std::endl;
        return;
    }

    float dx = n1.x - n2.x;
    float dy = n1.y - n2.y;
    float distance = sqrt(dx * dx + dy * dy);

    int edgeId = getNextEdgeId(graph);

    int speedLimit = isOverpass ? 80 : 60;

    graph.addEdge(edgeId, node1, node2, distance, speedLimit,
        (isOverpass ? "Overpass" : "Bridge"));

    std::cout << (isOverpass ? "Overpass" : "Bridge")
        << " added with ID " << edgeId << ", length: "
        << distance << ", speed: " << speedLimit << " km/h" << std::endl;
}

void MapGenerator::generateHighwayNetwork(Graph& graph) {
    std::cout << "Generating highway network..." << std::endl;

    int baseNodeId = getNextNodeId(graph);
    int baseEdgeId = getNextEdgeId(graph);

    std::vector<int> highwayNodes;

    for (int i = 0; i < 5; i++) {
        int nodeId = baseNodeId++;
        float x = 100.0f + i * 200.0f;
        float y = 100.0f;

        graph.addNode(nodeId, x, y, "Highway-" + std::to_string(i));
        highwayNodes.push_back(nodeId);
    }

    for (int i = 0; i < 4; i++) {
        graph.addEdge(baseEdgeId++, highwayNodes[i], highwayNodes[i + 1],
            200.0f, 100, "Highway Segment");
    }

    std::cout << "Highway network created with 5 nodes and 4 edges" << std::endl;
}

void MapGenerator::generateRadialCity(Graph& graph) {
    std::cout << "Generating WELL-CONNECTED RADIAL CITY..." << std::endl;

    int nodeId = 1;
    int edgeId = 1;

    graph.addNode(nodeId, 400.0f, 300.0f, "City Center");
    int centerId = nodeId;
    nodeId++;

    int rings = 4;
    int spokes = 12;  

    std::vector<std::vector<int>> ringNodes(rings);

    for (int ring = 0; ring < rings; ring++) {
        float radius = 60.0f + (ring * 70.0f);  

        for (int spoke = 0; spoke < spokes; spoke++) {
            float angle = spoke * (360.0f / spokes) * 3.14159f / 180.0f;
            float x = 400.0f + radius * cos(angle);
            float y = 300.0f + radius * sin(angle);

            std::string name = "R" + std::to_string(ring + 1) +
                "-S" + std::to_string(spoke + 1);
            graph.addNode(nodeId, x, y, name);
            ringNodes[ring].push_back(nodeId);
            nodeId++;

            if (ring == 0) {  
                Node center = graph.getNode(centerId);
                Node node = graph.getNode(nodeId - 1);
                float dist = sqrt(pow(center.x - node.x, 2) +
                    pow(center.y - node.y, 2));
                graph.addEdge(edgeId++, centerId, nodeId - 1, dist, 40, "Main Spoke");
            }
        }
    }

    for (int spoke = 0; spoke < spokes; spoke++) {
        for (int ring = 0; ring < rings - 1; ring++) {
            int fromNode = ringNodes[ring][spoke];
            int toNode = ringNodes[ring + 1][spoke];

            float dist = 70.0f;  

            graph.addEdge(edgeId++, fromNode, toNode, dist, 50,
                "Radial " + std::to_string(spoke + 1));
        }
    }

    for (int ring = 0; ring < rings; ring++) {
        for (int spoke = 0; spoke < spokes; spoke++) {
            int fromNode = ringNodes[ring][spoke];
            int toNode = ringNodes[ring][(spoke + 1) % spokes];

            float radius = 60.0f + (ring * 70.0f);
            float arcLength = 2.0f * 3.14159f * radius / spokes;

            graph.addEdge(edgeId++, fromNode, toNode, arcLength, 50,
                "Ring " + std::to_string(ring + 1) + " Road");
        }
    }

    for (int ring = 0; ring < rings - 1; ring++) {
        for (int spoke = 0; spoke < spokes; spoke += 2) {
            int fromNode = ringNodes[ring][spoke];
            int toNode = ringNodes[ring + 1][(spoke + 1) % spokes];

            Node n1 = graph.getNode(fromNode);
            Node n2 = graph.getNode(toNode);
            float dx = n1.x - n2.x;
            float dy = n1.y - n2.y;
            float dist = sqrt(dx * dx + dy * dy);

            graph.addEdge(edgeId++, fromNode, toNode, dist, 45, "Diagonal");
        }
    }

    for (int spoke = 0; spoke < spokes; spoke += 3) {
        int outerNode = ringNodes[rings - 1][spoke];

        Node center = graph.getNode(centerId);
        Node outer = graph.getNode(outerNode);
        float dist = sqrt(pow(center.x - outer.x, 2) +
            pow(center.y - outer.y, 2));

        graph.addEdge(edgeId++, centerId, outerNode, dist, 70, "Expressway");
    }

    std::cout << "✅ Radial city generated with perfect connectivity!" << std::endl;
    std::cout << "   - " << (nodeId - 1) << " nodes" << std::endl;
    std::cout << "   - " << (edgeId - 1) << " edges" << std::endl;
    std::cout << "   - Center connected to all areas" << std::endl;
    std::cout << "   - All rings interconnected" << std::endl;
}



void MapGenerator::generateOrganicCity(Graph& graph) {
    std::cout << "Generating ORGANIC CITY layout..." << std::endl;

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    int baseNodeId = getNextNodeId(graph);
    int baseEdgeId = getNextEdgeId(graph);

    std::vector<std::pair<float, float>> clusters = {
        {200.0f, 200.0f},  
        {600.0f, 200.0f},  
        {400.0f, 400.0f},  
        {300.0f, 600.0f},  
        {700.0f, 500.0f}   
    };

    std::vector<int> allNodes;

    for (int c = 0; c < clusters.size(); c++) {
        float centerX = clusters[c].first;
        float centerY = clusters[c].second;

        int nodesInCluster = 3 + (std::rand() % 3);

        for (int n = 0; n < nodesInCluster; n++) {
            float x = centerX + (std::rand() % 100 - 50);
            float y = centerY + (std::rand() % 100 - 50);
            graph.addNode(baseNodeId, x, y, "C" + std::to_string(c) + "-" + std::to_string(n));
            allNodes.push_back(baseNodeId);
            baseNodeId++;
        }
    }

    for (size_t i = 0; i < allNodes.size(); i++) {
        int connections = 2 + (std::rand() % 2);

        for (int c = 0; c < connections && (i + c + 1) < allNodes.size(); c++) {
            int from = allNodes[i];
            int to = allNodes[i + c + 1];

            Node n1 = graph.getNode(from);
            Node n2 = graph.getNode(to);

            float dx = n1.x - n2.x;
            float dy = n1.y - n2.y;
            float dist = sqrt(dx * dx + dy * dy);

            graph.addEdge(baseEdgeId++, from, to, dist, 30 + (std::rand() % 40), "Local");
        }
    }
}

void MapGenerator::generateRandomCity(Graph& graph, int nodeCount) {
    std::cout << "Generating random city with " << nodeCount << " nodes..." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distX(100.0f, 700.0f);
    std::uniform_real_distribution<> distY(100.0f, 700.0f);
    std::uniform_int_distribution<> distSpeed(30, 80);

    int baseNodeId = getNextNodeId(graph);
    int baseEdgeId = getNextEdgeId(graph);

    std::vector<int> nodeIds;
    for (int i = 0; i < nodeCount; i++) {
        int nodeId = baseNodeId++;
        float x = distX(gen);
        float y = distY(gen);
        graph.addNode(nodeId, x, y, "Random-" + std::to_string(i));
        nodeIds.push_back(nodeId);
    }

    for (size_t i = 0; i < nodeIds.size(); i++) {
        std::vector<std::pair<float, int>> distances;

        for (size_t j = 0; j < nodeIds.size(); j++) {
            if (i == j) continue;

            Node node1 = graph.getNode(nodeIds[i]);
            Node node2 = graph.getNode(nodeIds[j]);

            float dx = node1.x - node2.x;
            float dy = node1.y - node2.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < 200.0f) { 
                distances.push_back({ distance, nodeIds[j] });
            }
        }

        std::sort(distances.begin(), distances.end());
        int connections = std::min(static_cast<int>(distances.size()),
            std::uniform_int_distribution<>(2, 4)(gen));

        for (int k = 0; k < connections; k++) {
            int toNode = distances[k].second;
            float distance = distances[k].first;
            int speedLimit = distSpeed(gen);

            bool edgeExists = false;
            auto existingEdges = graph.getEdgesFromNode(nodeIds[i]);
            for (int edgeId : existingEdges) {
                Edge edge = graph.getEdge(edgeId);
                if ((edge.fromNodeId == nodeIds[i] && edge.toNodeId == toNode) ||
                    (edge.fromNodeId == toNode && edge.toNodeId == nodeIds[i])) {
                    edgeExists = true;
                    break;
                }
            }

            if (!edgeExists) {
                graph.addEdge(baseEdgeId++, nodeIds[i], toNode, distance, speedLimit,
                    "Random Road " + std::to_string(nodeIds[i]) + "-" + std::to_string(toNode));
            }
        }
    }
}

void MapGenerator::connectGrid(Graph& graph, int rows, int cols,
    float startX, float startY, float spacing) {

    std::cout << "Connecting grid of " << rows << "x" << cols
        << " starting at (" << startX << ", " << startY
        << ") with spacing " << spacing << std::endl;

    int baseEdgeId = getNextEdgeId(graph);

    std::vector<std::vector<int>> nodeGrid(rows, std::vector<int>(cols, -1));

    auto allNodes = graph.getAllNodes();

    for (const auto& pair : allNodes) {
        const Node& node = pair.second;

        if (node.x >= startX && node.x <= startX + (cols - 1) * spacing &&
            node.y >= startY && node.y <= startY + (rows - 1) * spacing) {

            int col = static_cast<int>((node.x - startX) / spacing + 0.5f);
            int row = static_cast<int>((node.y - startY) / spacing + 0.5f);

            if (row >= 0 && row < rows && col >= 0 && col < cols) {
                nodeGrid[row][col] = node.id;
                std::cout << "  Node " << node.id << " placed at grid ["
                    << row << "][" << col << "]" << std::endl;
            }
        }
    }

    int edgesCreated = 0;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols - 1; col++) {
            int fromNode = nodeGrid[row][col];
            int toNode = nodeGrid[row][col + 1];

            if (fromNode != -1 && toNode != -1) {
                bool edgeExists = false;
                auto existingEdges = graph.getEdgesFromNode(fromNode);

                for (int edgeId : existingEdges) {
                    Edge edge = graph.getEdge(edgeId);
                    if ((edge.fromNodeId == fromNode && edge.toNodeId == toNode) ||
                        (edge.fromNodeId == toNode && edge.toNodeId == fromNode)) {
                        edgeExists = true;
                        break;
                    }
                }

                if (!edgeExists) {
                    int speedLimit = 50; 
                    if (row == 0 || row == rows - 1) speedLimit = 70; 

                    graph.addEdge(baseEdgeId++, fromNode, toNode, spacing, speedLimit,
                        "Grid-H-" + std::to_string(row) + "-" + std::to_string(col));

                    edgesCreated++;
                    std::cout << "  Connected " << fromNode << " → " << toNode
                        << " (horizontal, speed: " << speedLimit << " km/h)" << std::endl;
                }
            }
        }
    }

    for (int col = 0; col < cols; col++) {
        for (int row = 0; row < rows - 1; row++) {
            int fromNode = nodeGrid[row][col];
            int toNode = nodeGrid[row + 1][col];

            if (fromNode != -1 && toNode != -1) {
                bool edgeExists = false;
                auto existingEdges = graph.getEdgesFromNode(fromNode);

                for (int edgeId : existingEdges) {
                    Edge edge = graph.getEdge(edgeId);
                    if ((edge.fromNodeId == fromNode && edge.toNodeId == toNode) ||
                        (edge.fromNodeId == toNode && edge.toNodeId == fromNode)) {
                        edgeExists = true;
                        break;
                    }
                }

                if (!edgeExists) {
                    int speedLimit = 50; 
                    if (col == 0 || col == cols - 1) speedLimit = 70; 

                    graph.addEdge(baseEdgeId++, fromNode, toNode, spacing, speedLimit,
                        "Grid-V-" + std::to_string(row) + "-" + std::to_string(col));

                    edgesCreated++;
                    std::cout << "  Connected " << fromNode << " → " << toNode
                        << " (vertical, speed: " << speedLimit << " km/h)" << std::endl;
                }
            }
        }
    }

    for (int row = 0; row < rows - 1; row++) {
        for (int col = 0; col < cols - 1; col++) {
            int fromNode = nodeGrid[row][col];
            int toNode = nodeGrid[row + 1][col + 1];

            if (fromNode != -1 && toNode != -1) {
                bool edgeExists = false;
                auto existingEdges = graph.getEdgesFromNode(fromNode);

                for (int edgeId : existingEdges) {
                    Edge edge = graph.getEdge(edgeId);
                    if ((edge.fromNodeId == fromNode && edge.toNodeId == toNode) ||
                        (edge.fromNodeId == toNode && edge.toNodeId == fromNode)) {
                        edgeExists = true;
                        break;
                    }
                }

                if (!edgeExists) {
                    float diagonalDist = spacing * std::sqrt(2.0f);

                    graph.addEdge(baseEdgeId++, fromNode, toNode, diagonalDist, 40,
                        "Grid-D-" + std::to_string(row) + "-" + std::to_string(col));

                    edgesCreated++;
                    std::cout << "  Connected " << fromNode << " → " << toNode
                        << " (diagonal)" << std::endl;
                }
            }
        }
    }

    std::cout << "Grid connections complete: " << edgesCreated << " new edges created" << std::endl;
}

void MapGenerator::generateCoastalStyleCity(Graph& graph) {
    std::cout << "Generating COASTAL-STYLE CITY..." << std::endl;

    int nodeId = 1;
    int edgeId = 1;

    std::vector<int> coastNodes;
    for (int i = 0; i < 8; i++) {
        float x = 150.0f + i * 80.0f;
        float y = 150.0f + 50.0f * sin(i * 0.8f); 
        graph.addNode(nodeId, x, y, "Coast-" + std::to_string(i));
        coastNodes.push_back(nodeId);
        nodeId++;
    }

    for (size_t i = 0; i < coastNodes.size() - 1; i++) {
        Node n1 = graph.getNode(coastNodes[i]);
        Node n2 = graph.getNode(coastNodes[i + 1]);
        float dx = n1.x - n2.x;
        float dy = n1.y - n2.y;
        float dist = sqrt(dx * dx + dy * dy);
        graph.addEdge(edgeId++, coastNodes[i], coastNodes[i + 1], dist, 60, "Coast Road");
    }

    int gridNodes[4][4];
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float x = 200.0f + col * 90.0f;
            float y = 250.0f + row * 80.0f;
            graph.addNode(nodeId, x, y, "Inland-" + std::to_string(row) + "-" + std::to_string(col));
            gridNodes[row][col] = nodeId;
            nodeId++;
        }
    }

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            graph.addEdge(edgeId++, gridNodes[row][col], gridNodes[row][col + 1], 90.0f, 50, "Street");
        }
    }

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 3; row++) {
            graph.addEdge(edgeId++, gridNodes[row][col], gridNodes[row + 1][col], 80.0f, 50, "Avenue");
        }
    }

    graph.addEdge(edgeId++, coastNodes[2], gridNodes[0][0], 120.0f, 60, "Bridge 1");
    graph.addEdge(edgeId++, coastNodes[5], gridNodes[0][3], 120.0f, 60, "Bridge 2");

    std::cout << "Coastal city generated with " << (nodeId - 1) << " nodes" << std::endl;
}

void MapGenerator::generateNextCity(Graph& graph) {
    std::cout << "\n GENERATING NEXT CITY TYPE..." << std::endl;

    std::cout << "Clearing old city..." << std::endl;
    graph.clearGraph();  // Fixed: use clearGraph() instead of clearAll()

    std::cout << "Graph cleared. Node count: " << graph.getNodeCount()
        << ", Edge count: " << graph.getEdgeCount() << std::endl;

    currentCityType = static_cast<CityType>((static_cast<int>(currentCityType) + 1)
        % static_cast<int>(CityType::CITY_TYPE_COUNT));

    std::cout << "Selected city type: ";

    switch (currentCityType) {
    case CityType::SIMPLE_GRID:
        std::cout << "SIMPLE GRID (6x6)" << std::endl;
        generateSimpleGrid(graph, 6);
        break;

    case CityType::COMPLEX_CITY:
        std::cout << "COMPLEX CITY (Mixed Zones)" << std::endl;
        generateComplexCity(graph);
        break;

    case CityType::RANDOM_CITY:
        std::cout << "RANDOM CITY (Organic)" << std::endl;
        generateRandomCity(graph, 25);
        break;

    case CityType::METROPOLIS:
        std::cout << "METROPOLIS (Urban Center)" << std::endl;
        generateSimpleGrid(graph, 10); 
        break;

    case CityType::PLANNED_CITY:
        std::cout << "PLANNED CITY (Organized Grid)" << std::endl;
        generateSimpleGrid(graph, 8); 
        break;

    case CityType::COASTAL_CITY:
        std::cout << "COASTAL CITY (Waterfront)" << std::endl;
        generateCoastalStyleCity(graph);
        break;

    default:
        std::cout << "DEFAULT (Simple Grid)" << std::endl;
        generateSimpleGrid(graph, 6);
    }

    std::cout << "New city generated! Nodes: " << graph.getNodeCount()
        << ", Edges: " << graph.getEdgeCount() << std::endl;
}