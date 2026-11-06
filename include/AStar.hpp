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

/// Which number the open set is sorted by. Only the priority changes between strategies.
enum class SearchStrategy {
    AStar,         ///< f = g + h  (optimal with an admissible heuristic)
    Dijkstra,      ///< f = g      (no heuristic: h is reported as 0)
    Greedy,        ///< f = h      (ignores the cost already paid)
    WeightedAStar, ///< f = g + 2h (trusts the heuristic more)
};

const char* strategyName(SearchStrategy strategy);

struct SearchResult {
    bool found{false};
    int cost{0};
    std::vector<NodeScore> expanded;
    std::vector<int> path;
    std::vector<SearchStep> steps;
};

class AStar {
public:
    SearchResult search(const Graph& graph, int start, int goal,
                        SearchStrategy strategy = SearchStrategy::AStar) const;

private:
    static int heuristic(const GraphNode& from, const GraphNode& to);
};

