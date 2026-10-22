#pragma once

#include "Graph.hpp"

#include <vector>

struct NodeScore {
    int nodeId{};
    int g{};
    int h{};
    int f{};
};

enum class SearchStepType {
    Initialize,
    BeginLoop,
    SelectCurrent,
    CheckGoal,
    InspectNeighbor,
    CalculateTentative,
    CompareCost,
    UpdateNeighbor,
    GoalReached,
    SearchExhausted,
};

struct SearchStep {
    SearchStepType type{SearchStepType::Initialize};
    int pseudocodeLine{1};
    int currentNode{-1};
    int neighborNode{-1};
    int edgeCost{0};
    int tentativeG{0};
    int previousG{-1};
    bool improved{false};
    NodeScore score{};
};

struct SearchResult {
    bool found{false};
    int cost{0};
    std::vector<NodeScore> expanded;
    std::vector<int> path;
    std::vector<SearchStep> steps;
};

class AStar {
public:
    SearchResult search(const Graph& graph, int start, int goal) const;

private:
    static int heuristic(const GraphNode& from, const GraphNode& to);
};

