#include "AStar.hpp"

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}
} // namespace

int main() {
    AStar astar;

    {
        Graph graph;
        graph.addNode(0.f, 0.f);
        graph.addNode(40.f, 20.f);
        graph.addNode(100.f, 0.f);
        graph.addUndirectedEdge(0, 2, 12);
        graph.addUndirectedEdge(0, 1, 4);
        graph.addUndirectedEdge(1, 2, 5);
        const auto result = astar.search(graph, 0, 2);
        require(result.found, "a connected graph should have a path");
        require(result.cost == 9, "A* should choose the cheaper weighted route");
        require(result.path.size() == 3 && result.path[1] == 1,
                "the optimal path should pass through the cheaper node");
        require(!result.steps.empty(), "the search should provide educational trace steps");
        require(result.steps.front().type == SearchStepType::Initialize,
                "the trace should begin by initializing the open set");
        require(result.steps.back().type == SearchStepType::GoalReached,
                "a successful trace should end by rebuilding the path");
        bool recordedUpdate = false;
        for (const auto& step : result.steps) {
            if (step.type == SearchStepType::UpdateNeighbor && step.neighborNode == 1) {
                recordedUpdate = true;
            }
        }
        require(recordedUpdate, "the trace should record neighbor cost updates");
    }

    {
        Graph graph;
        graph.addNode(0.f, 0.f);
        graph.addNode(50.f, 0.f);
        graph.addNode(100.f, 0.f);
        graph.addUndirectedEdge(0, 1, 5);
        graph.addUndirectedEdge(1, 2, 5);
        graph.edges()[1].enabled = false;
        const auto result = astar.search(graph, 0, 2);
        require(!result.found, "disabled edges should make a disconnected goal unreachable");
        require(!result.steps.empty() &&
                    result.steps.back().type == SearchStepType::SearchExhausted,
                "a failed trace should show that the open set was exhausted");
    }

    {
        Graph graph;
        graph.addNode(0.f, 0.f);
        graph.addNode(50.f, 0.f);
        const auto result = astar.search(graph, 0, 0);
        require(result.found && result.cost == 0, "start equal to goal should cost zero");
    }

    std::cout << "All AStarLab tests passed.\n";
    return 0;
}
