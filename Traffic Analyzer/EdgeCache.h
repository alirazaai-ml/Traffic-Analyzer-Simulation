#pragma once
#include <unordered_map>
#include <utility>

// Custom hash function for pair<int, int>
struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        // Combine hashes using XOR and bit shifting
        return hash1 ^ (hash2 << 1);
    }
};

// Edge lookup cache for O(1) edge finding by node pair
class EdgeCache {
private:
    // Maps (fromNode, toNode) -> edgeId
    std::unordered_map<std::pair<int, int>, int, PairHash> cache;
    bool isDirty;

public:
    EdgeCache() : isDirty(true) {}

    // Add edge to cache
    void addEdge(int fromNode, int toNode, int edgeId) {
        cache[{fromNode, toNode}] = edgeId;
        cache[{toNode, fromNode}] = edgeId; // Bidirectional
    }

    // Find edge ID by node pair
    int findEdge(int fromNode, int toNode) const {
        auto it = cache.find({fromNode, toNode});
        if (it != cache.end()) {
            return it->second;
        }
        return -1; // Not found
    }

    // Clear cache
    void clear() {
        cache.clear();
        isDirty = true;
    }

    // Check if cache is dirty
    bool needsRebuild() const {
        return isDirty;
    }

    // Mark cache as clean
    void markClean() {
        isDirty = false;
    }

    // Get cache statistics
    size_t size() const {
        return cache.size() / 2; // Divide by 2 because of bidirectional entries
    }
};
