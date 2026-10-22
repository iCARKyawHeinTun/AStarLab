#pragma once

#include <utility>
#include <vector>

struct GraphNode {
    int id{};
    float x{};
    float y{};
};

struct GraphEdge {
    int from{};
    int to{};
    int cost{};
    bool enabled{true};
};

class Graph {
public:
    void clear();
    int addNode(float x, float y);
    void addUndirectedEdge(int from, int to, int cost);
    const GraphNode& node(int id) const;
    const std::vector<GraphNode>& nodes() const { return nodes_; }
    const std::vector<GraphEdge>& edges() const { return edges_; }
    std::vector<GraphEdge>& edges() { return edges_; }
    std::vector<std::pair<int, int>> neighbors(int nodeId) const;

private:
    std::vector<GraphNode> nodes_;
    std::vector<GraphEdge> edges_;
};

