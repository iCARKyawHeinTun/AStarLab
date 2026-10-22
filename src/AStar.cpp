#include "AStar.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace {
struct QueueEntry {
    int nodeId{};
    int f{};
    int sequence{};
};

struct LowerScoreFirst {
    bool operator()(const QueueEntry& left, const QueueEntry& right) const {
        if (left.f == right.f) return left.sequence > right.sequence;
        return left.f > right.f;
    }
};
} // namespace

SearchResult AStar::search(const Graph& graph, int start, int goal) const {
    SearchResult result;
    if (start < 0 || goal < 0 || start >= static_cast<int>(graph.nodes().size()) ||
        goal >= static_cast<int>(graph.nodes().size())) {
        return result;
    }

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, LowerScoreFirst> frontier;
    std::unordered_map<int, int> gScore;
    std::unordered_map<int, int> cameFrom;
    std::unordered_set<int> closed;
    int sequence = 0;

    gScore[start] = 0;
    const int startHeuristic = heuristic(graph.node(start), graph.node(goal));
    frontier.push({start, startHeuristic, sequence++});
    result.steps.push_back({SearchStepType::Initialize, 1, start, -1, 0, 0, -1, true,
                            {start, 0, startHeuristic, startHeuristic}});

    while (!frontier.empty()) {
        result.steps.push_back({SearchStepType::BeginLoop, 2});
        const int current = frontier.top().nodeId;
        frontier.pop();
        if (closed.count(current) != 0U) continue;

        const int h = heuristic(graph.node(current), graph.node(goal));
        const NodeScore currentScore{current, gScore[current], h, gScore[current] + h};
        closed.insert(current);
        result.expanded.push_back(currentScore);
        result.steps.push_back({SearchStepType::SelectCurrent, 3, current, -1, 0, 0, -1,
                                false, currentScore});
        result.steps.push_back({SearchStepType::CheckGoal, 4, current, -1, 0, 0, -1,
                                current == goal, currentScore});

        if (current == goal) {
            result.found = true;
            result.cost = gScore[current];
            int step = goal;
            result.path.push_back(step);
            while (step != start) {
                step = cameFrom.at(step);
                result.path.push_back(step);
            }
            std::reverse(result.path.begin(), result.path.end());
            result.steps.push_back({SearchStepType::GoalReached, 9, current, -1, 0,
                                    gScore[current], -1, true, currentScore});
            return result;
        }

        for (const auto [neighbor, edgeCost] : graph.neighbors(current)) {
            if (closed.count(neighbor) != 0U) continue;
            result.steps.push_back({SearchStepType::InspectNeighbor, 5, current, neighbor,
                                    edgeCost, 0, -1, false, currentScore});
            const int tentative = gScore[current] + edgeCost;
            const auto known = gScore.find(neighbor);
            const int previous = known == gScore.end() ? -1 : known->second;
            result.steps.push_back({SearchStepType::CalculateTentative, 6, current, neighbor,
                                    edgeCost, tentative, previous, false, currentScore});
            const bool improved = known == gScore.end() || tentative < known->second;
            result.steps.push_back({SearchStepType::CompareCost, 7, current, neighbor,
                                    edgeCost, tentative, previous, improved, currentScore});
            if (improved) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative;
                const int estimate = heuristic(graph.node(neighbor), graph.node(goal));
                const NodeScore neighborScore{neighbor, tentative, estimate, tentative + estimate};
                frontier.push({neighbor, tentative + estimate, sequence++});
                result.steps.push_back({SearchStepType::UpdateNeighbor, 8, current, neighbor,
                                        edgeCost, tentative, previous, true, neighborScore});
            }
        }
    }
    result.steps.push_back({SearchStepType::SearchExhausted, 2});
    return result;
}

int AStar::heuristic(const GraphNode& from, const GraphNode& to) {
    const float dx = from.x - to.x;
    const float dy = from.y - to.y;
    return static_cast<int>(std::floor(std::sqrt(dx * dx + dy * dy) / 10.f));
}

