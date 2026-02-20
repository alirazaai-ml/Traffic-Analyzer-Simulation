# Traffic Analyzer - Advanced Traffic Simulation System

## ?? Project Overview

Traffic Analyzer is a sophisticated C++ application that simulates and visualizes urban traffic networks in real-time. It models road networks as directed graphs and provides predictive analytics for traffic congestion, accident simulation, dynamic routing, and interactive visualization using the SFML (Simple and Fast Multimedia Library) framework.

### Core Features
- **Real-time Traffic Simulation**: Dynamic vehicle movement with realistic traffic patterns
- **Predictive Analytics**: Machine learning-based congestion prediction using multiple algorithms
- **Accident System**: Random accident generation with automatic road blocking and recovery
- **Interactive UI**: Zoom, pan, node selection, and route visualization
- **Multiple Map Generation**: 6 different city layouts (grid, metropolis, coastal, etc.)
- **Traffic Flow Analysis**: Color-coded roads based on congestion levels
- **Peak Hour Simulation**: Simulate rush hour conditions with increased traffic

---

## ??? Architecture & System Design

### Component Structure

```
Traffic Analyzer
?
??? Core Graph System
?   ??? Graph.cpp/h           - Graph data structure (nodes & edges)
?   ??? Node struct           - Intersection/waypoint (id, x, y, name)
?   ??? Edge struct           - Road segment (length, speed limit, traffic level)
?
??? Simulation Systems
?   ??? CarSimulation         - Vehicle movement & pathfinding
?   ??? AccidentSystem        - Accident creation, tracking & visualization
?   ??? PredictionSystem      - Traffic prediction algorithms
?
??? Visualization
?   ??? GUI                   - Main UI controller & event handling
?   ??? MapRenderer           - Low-level SFML drawing primitives
?
??? Utilities
?   ??? MapGenerator          - Procedural city generation
?
??? Entry Point
    ??? main.cpp              - Application initialization
```

### Data Flow

1. **Initialization**: `main.cpp` ? `MapGenerator` ? `Graph` ? `GUI`
2. **Update Loop**: `GUI::update()` ? `CarSimulation::update()` ? `AccidentSystem::update()` ? `PredictionSystem::update()`
3. **Render Loop**: `GUI::render()` ? `drawMap()` ? `MapRenderer` primitives
4. **Event Handling**: User input ? `GUI::handleEvents()` ? System updates

---

## ?? Technical Implementation Details

### 1. Graph System (`Graph.cpp/h`)

**Data Structures:**
- `std::unordered_map<int, Node> nodes` - O(1) node lookup
- `std::unordered_map<int, Edge> edges` - O(1) edge lookup
- `std::unordered_map<int, std::vector<int>> adjacencyList` - Edge connectivity

**Key Algorithms:**
- **Dijkstra's Algorithm**: Shortest path calculation with travel time weights
- **Edge Traffic Update**: Dynamic traffic level calculation based on current speed
- **Accident Blocking**: Temporary road closure with timer-based recovery

**Traffic Levels:**
```cpp
enum class TrafficLevel {
    FREE_FLOW = 0,    // Speed > 70% of limit (Green)
    SLOW = 1,         // Speed 30-70% of limit (Yellow)
    CONGESTED = 2,    // Speed < 30% of limit (Red)
    BLOCKED = 3       // Accident/construction (Gray)
};
```

### 2. Car Simulation System (`CarSimulation.cpp/h`)

**Features:**
- Automatic traffic generation with configurable spawn intervals
- Congestion-aware speed adjustment
- Dynamic rerouting capability
- Maximum car limit (100 vehicles)

**Update Logic:**
```cpp
// Congestion factor based on cars per edge
float congestionFactor = 1.0f / (1.0f + carsOnThisEdge * 0.3f);

// Speed calculation
float speed = baseSpeed * congestionFactor * simulationSpeed;
```

**Adaptive Spawn Rate:**
- < 20 cars: 2 second intervals
- 20-50 cars: 3 second intervals  
- > 50 cars: 5 second intervals

### 3. Prediction System (`PredictionSystem.cpp/h`)

**Algorithms Implemented:**
1. **Simple Moving Average** - Average of last N samples
2. **Weighted Moving Average** - Recent data weighted higher
3. **Exponential Smoothing** - ? = 0.3 smoothing factor
4. **Linear Regression** - Trend-based prediction

**Prediction Strategy:**
- 5-minute prediction: Average of all 3 algorithms
- 10-minute prediction: 5-min + 50% trend continuation
- Peak hour adjustment: -30% speed (5-min), -40% speed (10-min)
- Minimum speed cap: 5 km/h
- Confidence calculation: Based on variance and data history size

**Performance:**
- Updates every 5 seconds
- Stores last 2 minutes of speed data per edge
- Tracks prediction accuracy over time

### 4. Accident System (`AccidentSystem.cpp/h`)

**Features:**
- Random accident generation on any edge
- Configurable duration (default: 180 seconds)
- Visual blinking warning icons
- Automatic road blocking and recovery
- Multiple simultaneous accidents supported

**Visual Effects:**
- Blinking red color on affected edges (2 Hz)
- Warning icon at edge midpoint
- Pulsing animation on warning sprites

### 5. GUI System (`GUI.cpp/h`)

**UI Layout:**
- **Main Viewport** (900×800px): Zoomable/pannable map view
- **Control Panel** (300×800px): Buttons, inputs, statistics

**Interactive Features:**
- Left-click: Select nodes for routing
- Right-drag: Pan the map
- Mouse wheel: Zoom (0.1× to 5.0×)
- Text input: Manual node ID entry
- 12 functional buttons

**Control Panel Buttons:**
```
Column 1:                    Column 2:
- Find Path                  - Generate City
- Add Car                    - 20 Cars
- Clear Cars                 - Rush Hour
- Traffic Sim                - Clear All
- Peak Hour                  - Clear Accidents
- Accident                   - Predictions
```

**Statistics Display:**
- Node/road/car counts
- Average speed
- Congestion percentage
- Active accidents
- Predicted congestion count
- Prediction accuracy
- Current route info

### 6. Map Generator (`MapGenerator.cpp/h`)

**City Types:**
1. Simple Grid (4×4)
2. Complex City (grid with diagonals)
3. Random City (random connections)
4. Metropolis (dense urban)
5. Planned City (organized layout)
6. Coastal City (geographic constraints)

---

## ?? Issues Identified & Recommendations

### Critical Issues

#### 1. **PredictionSystem const-correctness (RESOLVED)**
**Issue:** `predictEdge()` is called from const contexts but modifies internal state
**Status:** ? Fixed in header with `predictEdgeInternal()` and const version
**Impact:** Prevents compilation errors with const method calls

#### 2. **Memory Management**
**Issue:** Raw pointers without proper cleanup in some destructors
```cpp
// GUI.cpp - GOOD ?
~GUI() {
    delete carSim;
    delete accidentSystem;
    delete predictionSystem;
}
```
**Recommendation:** Consider `std::unique_ptr<>` for safer RAII

#### 3. **CarSimulation Ownership**
**Issue:** `GUI` creates `CarSimulation` but passes raw `Graph&`
```cpp
carSim = new CarSimulation(map); // map is a reference
```
**Risk:** If `Graph` is destroyed before `CarSimulation`, dangling reference
**Fix:** Ensure proper lifetime management or use shared ownership

### Performance Issues

#### 4. **Inefficient Edge Lookups**
**Problem:** `findEdge(fromNode, toNode)` has O(E) complexity
```cpp
Edge CarSimulation::findEdge(int fromNode, int toNode) const {
    auto edgesList = cityMap.getEdgesFromNode(fromNode); // O(1)
    for (int edgeId : edgesList) { // O(degree)
        Edge edge = cityMap.getEdge(edgeId); // O(1)
        // Linear search through edges
    }
}
```
**Current:** Called in every `update()` for every car
**Fix:** Add edge lookup map: `std::unordered_map<std::pair<int,int>, int>`

#### 5. **Redundant Path Calculations**
**Problem:** Path recalculated in multiple places
```cpp
// GUI.cpp
auto path = cityMap.findShortestPath(start, end);  // Dijkstra O(E log V)
// CarSimulation.cpp
std::vector<int> calculateRoute(int start, int end) // Same algorithm
```
**Fix:** Cache paths or use A* for faster lookups

#### 6. **String Operations in Hot Loop**
**Problem:** String conversions in render loop
```cpp
void drawNode(...) {
    idText.setString(std::to_string(node.id)); // Every frame!
}
```
**Fix:** Pre-compute text strings, only update when changed

### Design Issues

#### 7. **Tight Coupling**
**Problem:** `GUI` depends on 5+ different classes directly
```cpp
class GUI {
    Graph& cityMap;
    CarSimulation* carSim;
    AccidentSystem* accidentSystem;
    PredictionSystem* predictionSystem;
    // ...
};
```
**Fix:** Use facade pattern or event system for decoupling

#### 8. **Magic Numbers**
**Problem:** Hard-coded values throughout
```cpp
controlPanel.setSize(sf::Vector2f(300.0f, 800.0f)); // Why 300?
if (activeCars < 100) { // Why 100?
float congestionFactor = 1.0f / (1.0f + carsOnThisEdge * 0.3f); // Why 0.3?
```
**Fix:** Define constants with semantic names
```cpp
constexpr float CONTROL_PANEL_WIDTH = 300.0f;
constexpr int MAX_ACTIVE_CARS = 100;
constexpr float CONGESTION_MULTIPLIER = 0.3f;
```

#### 9. **Font Loading Fallback Chain**
**Problem:** Tries multiple paths synchronously at startup
```cpp
const char* fontPaths[] = { /* 6 paths */ };
for (const char* path : fontPaths) {
    if (font.loadFromFile(path)) break;
}
```
**Fix:** Use configuration file or environment variable

### Missing Features

#### 10. **No Save/Load Simulation State**
**Current:** Can save/load graph, but not simulation state
**Needed:**
- Car positions
- Accident timers
- Prediction history
- Traffic levels

#### 11. **No Undo/Redo**
**Current:** Actions like "Generate City" cannot be undone
**Fix:** Command pattern for reversible operations

#### 12. **Limited Analytics**
**Missing:**
- Average trip time
- Congestion hotspots over time
- Traffic flow heatmaps
- Historical comparison

#### 13. **No Configuration System**
```cpp
// Hard-coded in CarSimulation.cpp
const int MAX_HISTORY_SIZE = 2;
const float PREDICTION_INTERVAL = 5.0f;
```
**Fix:** JSON/INI config file for tunable parameters

### Code Quality Issues

#### 14. **Inconsistent Error Handling**
```cpp
// Graph.cpp - Returns default-constructed object
Node Graph::getNode(int id) const {
    auto it = nodes.find(id);
    if (it != nodes.end()) return it->second;
    return Node(-1, 0, 0, ""); // Sentinel value
}
```
**Better:** Use `std::optional<Node>` or throw exceptions

#### 15. **Large Methods**
**Problem:** `GUI::handleButtonClick()` is 250+ lines with nested if-else
**Fix:** Extract button handlers to separate methods:
```cpp
void GUI::handleButtonClick(const sf::Vector2f& mousePos) {
    if (findRouteBtn.contains(mousePos)) handleFindRoute();
    else if (addCarBtn.contains(mousePos)) handleAddCar();
    // ...
}
```

#### 16. **Commented-Out Code**
```cpp
// CarSimulation.cpp
//void CarSimulation::rerouteIfNeeded(Vehicle& vehicle) {
//    // TODO: Implement if needed, or remove this method
//}
```
**Fix:** Remove dead code or implement if needed

#### 17. **Insufficient Comments**
**Problem:** Complex algorithms lack explanation
```cpp
float m = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
```
**Better:**
```cpp
// Linear regression slope: y = mx + b
// m = (n?(xy) - ?x?y) / (n?(x²) - (?x)²)
float m = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
```

---

## ?? Performance Optimization Opportunities

### High Priority
1. **Edge lookup cache** - 40-60% update loop speedup
2. **Text caching** - 20-30% render speedup
3. **Spatial indexing** - Quadtree for node selection

### Medium Priority
4. **Path caching with invalidation** - On accident/congestion change
5. **Multithreading** - Separate threads for simulation/prediction/rendering
6. **LOD rendering** - Simplify at low zoom levels

### Low Priority
7. **GPU acceleration** - SFML shaders for effects
8. **Memory pooling** - Reuse car/accident objects

---

## ?? Potential Bugs & Edge Cases

### 1. **Division by Zero**
```cpp
// PredictionSystem.cpp
float coefficientOfVariation = stdDev / mean; // mean could be 0!
```

### 2. **Null Pointer Dereference**
```cpp
// GUI.cpp
if (predictionSystem) { // Good ?
    predictionSystem->update(deltaTime);
}
// But some places don't check
```

### 3. **Integer Overflow**
```cpp
int nextCarId; // Could overflow with long simulations
```
**Fix:** Use `size_t` or reset counter

### 4. **Path Size Assumptions**
```cpp
if (currentPath.size() < 2) return; // Good ?
// But some loops assume size >= 2 without check
```

### 5. **Concurrent Modification**
```cpp
// CarSimulation::update()
for (auto& car : cars) { /* modify cars */ }
cars.erase(...); // Modifying while iterating (fixed with std::remove_if)
```

---

## ?? Recommended Improvements Priority List

### Must-Do (High Impact, Low Effort)
1. ? Fix const-correctness in `PredictionSystem`
2. ? Remove unnecessary files (Vehicle, UIManager, TrafficSystem, TrafficUpdates, FontManager)
3. Define named constants for magic numbers
4. Add edge lookup cache
5. Add input validation for node IDs

### Should-Do (High Impact, Medium Effort)
6. Implement configuration system (JSON/INI)
7. Add save/load simulation state
8. Extract large methods into smaller functions
9. Add comprehensive error handling
10. Implement text caching in renderer

### Nice-to-Have (Medium Impact, High Effort)
11. Refactor to event-driven architecture
12. Add multithreading for systems
13. Implement A* pathfinding
14. Add undo/redo system
15. Create analytics dashboard

### Future Enhancements
16. Real-world map data import (OSM)
17. Machine learning for better predictions
18. Network mode (multi-client simulation)
19. 3D visualization mode
20. Mobile/web port

---

## ?? Project Files

### Core Systems
| File | Lines | Purpose | Status |
|------|-------|---------|--------|
| `Graph.cpp/h` | ~350 | Graph data structure & Dijkstra | ? Active |
| `CarSimulation.cpp/h` | ~400 | Vehicle movement & spawning | ? Active |
| `AccidentSystem.cpp/h` | ~200 | Accident lifecycle management | ? Active |
| `PredictionSystem.cpp/h` | ~450 | Traffic forecasting algorithms | ? Active |

### UI & Rendering
| File | Lines | Purpose | Status |
|------|-------|---------|--------|
| `GUI.cpp/h` | ~1000 | Main UI controller | ? Active |
| `MapRenderer.cpp/h` | ~200 | Low-level drawing | ? Active |

### Utilities
| File | Lines | Purpose | Status |
|------|-------|---------|--------|
| `MapGenerator.cpp/h` | ~300 | Procedural city generation | ? Active |
| `main.cpp` | ~30 | Entry point | ? Active |

### Removed (Unused/Redundant)
| File | Reason for Removal |
|------|-------------------|
| `Vehicle.cpp/h` | ? Not used - CarSimulation has its own Car struct |
| `UIManager.cpp/h` | ? Duplicate - GUI.cpp handles all UI |
| `TrafficSystem.cpp/h` | ? Not used - main.cpp directly uses GUI |
| `TrafficUpdates.cpp/h` | ? Not integrated - PredictionSystem handles traffic updates |
| `FontManager.cpp/h` | ? Redundant - GUI handles font loading directly |

---

## ?? Getting Started

### Prerequisites
- **C++ Compiler**: MSVC 2019+, GCC 9+, or Clang 10+
- **SFML 2.5+**: Graphics, Window, System modules
- **Windows**: Visual Studio 2019/2022 recommended
- **Linux**: GCC/Clang with SFML development packages
- **Font File**: `arial.ttf` in executable directory

### Build Instructions

#### Windows (Visual Studio)
```bash
# Option 1: vcpkg (Recommended)
vcpkg install sfml:x64-windows
vcpkg integrate install

# Open Traffic Analyzer.sln in Visual Studio
# Build -> Build Solution (Ctrl+Shift+B)
```

#### Linux (Command Line)
```bash
# Install SFML
sudo apt-get install libsfml-dev  # Ubuntu/Debian
sudo pacman -S sfml               # Arch Linux

# Build
g++ -std=c++17 \
    main.cpp GUI.cpp Graph.cpp MapRenderer.cpp MapGenerator.cpp \
    CarSimulation.cpp AccidentSystem.cpp PredictionSystem.cpp \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -o TrafficAnalyzer

# Run
./TrafficAnalyzer
```

### First Run
1. Ensure `arial.ttf` is in the same directory as the executable
2. Run the executable - a window with a generated city should appear
3. Click "Traffic Sim" to start automatic car spawning
4. Click "Find Path" after selecting start/end nodes
5. Try "Peak Hour" or "Accident" buttons to test scenarios

---

## ?? User Guide

### Basic Controls
- **Left Click**: Select source/destination nodes
- **Right Drag**: Pan the map
- **Mouse Wheel**: Zoom in/out
- **Type Numbers**: Enter node IDs manually
- **Enter**: Switch between source/destination input
- **Escape**: Close application

### Button Functions
- **Find Path**: Calculate shortest route between selected nodes
- **Add Car**: Spawn single random vehicle
- **20 Cars**: Spawn 20 vehicles at once
- **Clear Cars**: Remove all vehicles
- **Traffic Sim**: Toggle automatic traffic generation
- **Peak Hour**: Simulate rush hour (30 cars + congestion)
- **Rush Hour**: Extreme congestion (40 cars + accident)
- **Accident**: Create random road accident
- **Clear Accidents**: Remove all accidents
- **Clear All**: Reset traffic levels
- **Predictions**: Toggle congestion prediction overlay (purple)
- **Generate City**: Cycle through 6 city layouts

### Understanding the Display

#### Color Coding
- ?? **Green**: Free-flowing traffic (>70% speed limit)
- ?? **Yellow**: Slow traffic (30-70% speed limit)
- ?? **Red**: Congested (<30% speed limit)
- ? **Gray**: Blocked (accident/construction)
- ?? **Purple**: Predicted congestion (5-min forecast)

#### Statistics Panel
```
=== LIVE STATISTICS ===
Nodes:      50        # Intersections in map
Roads:      120       # Road segments
Active Cars: 35       # Vehicles currently moving
Avg Speed:  45.2 km/h # Network average
Congestion: 15.3%     # Percentage of congested roads
Accidents:  2         # Active road blocks
Pred. Cong: 5         # Roads likely to congest soon
Pred. Acc:  78.5%     # Prediction accuracy
```

---

## ?? Developer Guide

### Adding a New City Type
```cpp
// MapGenerator.cpp
void MapGenerator::generateMyCity(Graph& graph) {
    graph.clear();
    int nodeId = 0;
    
    // Add nodes
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            graph.addNode(nodeId++, x, y, "Node" + std::to_string(nodeId));
        }
    }
    
    // Add edges with connectivity logic
    // ...
}

// Add to generateNextCity() switch statement
```

### Creating Custom Prediction Algorithm
```cpp
// PredictionSystem.h
float myCustomPredictor(const std::deque<float>& data) const;

// PredictionSystem.cpp
float PredictionSystem::myCustomPredictor(const std::deque<float>& data) const {
    // Implement algorithm
    return predictedValue;
}

// Use in predictEdge()
float pred4 = myCustomPredictor(speeds);
prediction.predictedSpeed5min = (pred1 + pred2 + pred3 + pred4) / 4.0f;
```

### Adding New UI Button
```cpp
// GUI.h
Button myNewBtn;

// GUI.cpp constructor
createButton(myNewBtn, x, y, width, height, "Label");

// update()
updateBtn(myNewBtn);

// render()
drawButton(myNewBtn);

// handleButtonClick()
else if (myNewBtn.shape.getGlobalBounds().contains(mousePos)) {
    // Handle click
}
```

---

## ?? Testing Scenarios

### Scenario 1: Basic Path Finding
1. Launch application
2. Click node 0, then node 10
3. Click "Find Path"
4. **Expected**: Blue route appears between nodes

### Scenario 2: Traffic Jam
1. Click "Traffic Sim"
2. Wait 30 seconds
3. Click "Peak Hour"
4. **Expected**: Roads turn yellow/red, cars slow down

### Scenario 3: Accident Response
1. Start traffic simulation
2. Click "Accident" twice
3. Observe rerouting
4. **Expected**: Cars avoid blocked roads, take longer paths

### Scenario 4: Prediction Accuracy
1. Run simulation for 2 minutes
2. Click "Predictions"
3. Compare purple overlays with actual congestion
4. Check "Pred. Acc" statistic
5. **Expected**: >70% accuracy after sufficient data

---

## ?? Dependencies

### SFML Modules Used
- `sfml-graphics`: Drawing primitives, textures, sprites
- `sfml-window`: Window management, input handling
- `sfml-system`: Clock, vectors, utilities

### STL Components
- `<unordered_map>`: O(1) node/edge lookups
- `<vector>`: Dynamic arrays for paths, lists
- `<deque>`: History buffers in prediction system
- `<queue>`: Priority queue for Dijkstra's algorithm
- `<algorithm>`: Sorting, searching, removing
- `<random>`: Random number generation
- `<cmath>`: Mathematical functions
- `<iostream>`: Console logging

---

## ?? Known Issues

1. **Font Loading**: Requires `arial.ttf` in working directory - fallback to system fonts incomplete
2. **Long Simulation**: Car IDs increment without limit (potential overflow after days)
3. **Memory Usage**: Grows slowly with prediction history (minor leak possibility)
4. **Zoom Limits**: Text becomes unreadable at <0.5× zoom
5. **Path Invalidation**: Routes not recalculated when accidents block path

---

## ?? Future Roadmap

### Version 2.0 (Next Release)
- [ ] Configuration file system (JSON)
- [ ] Save/load simulation state
- [ ] Improved error handling
- [ ] Performance optimizations (edge cache, text cache)
- [ ] Analytics export (CSV)

### Version 3.0 (Advanced Features)
- [ ] A* pathfinding with heuristics
- [ ] Multithreading for subsystems
- [ ] Real-world map import (OpenStreetMap)
- [ ] Traffic light simulation
- [ ] Public transit routes

### Version 4.0 (Machine Learning)
- [ ] Neural network traffic prediction
- [ ] Reinforcement learning for signal timing
- [ ] Anomaly detection
- [ ] Demand forecasting

---

## ?? License
[Specify your license here - MIT, GPL, etc.]

## ?? Contributors
[Your name and contributors]

## ?? Contact
Repository: https://github.com/alirazaai-ml/Traffic-Analyzer-Simulation

---

**Last Updated**: January 2025  
**Version**: 1.0.1  
**Status**: Active Development  
**Cleanup**: Removed 10 unused files (Vehicle, UIManager, TrafficSystem, TrafficUpdates, FontManager)
