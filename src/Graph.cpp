#include "Graph.hpp"

#include <stdexcept>

void Graph::clear() {
    nodes_.clear();
    edges_.clear();
}

int Graph::addNode(float x, float y) {
    const int id = static_cast<int>(nodes_.size());
    nodes_.push_back({id, x, y});
    return id;
}

void Graph::addUndirectedEdge(int from, int to, int cost) {
    if (from < 0 || to < 0 || from >= static_cast<int>(nodes_.size()) ||
        to >= static_cast<int>(nodes_.size()) || from == to || cost <= 0) {
        throw std::invalid_argument("Invalid graph edge");
    }
    edges_.push_back({from, to, cost, true});
}

const GraphNode& Graph::node(int id) const {
    if (id < 0 || id >= static_cast<int>(nodes_.size())) {
        throw std::out_of_range("Invalid graph node");
    }
    return nodes_[static_cast<std::size_t>(id)];
}

std::vector<std::pair<int, int>> Graph::neighbors(int nodeId) const {
    std::vector<std::pair<int, int>> result;
    for (const auto& edge : edges_) {
        if (!edge.enabled) continue;
        if (edge.from == nodeId) result.emplace_back(edge.to, edge.cost);
        if (edge.to == nodeId) result.emplace_back(edge.from, edge.cost);
    }
    return result;
}

