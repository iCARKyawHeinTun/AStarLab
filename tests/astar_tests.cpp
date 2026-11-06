#include "AStar.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

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
        require(result.steps.back().pseudocodeLine == 5,
                "reaching the goal should highlight the return statement");
        bool recordedUpdate = false;
        for (const auto& step : result.steps) {
            if (step.type == SearchStepType::UpdateNeighbor && step.neighborNode == 1) {
                recordedUpdate = true;
                require(step.pseudocodeLine == 9,
                        "updating a neighbor should highlight the nested update statement");
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

    {
        // Two routes: one looks short but is expensive (via 1), one is cheaper (via 2).
        Graph graph;
        graph.addNode(0.f, 0.f);
        graph.addNode(60.f, 0.f);
        graph.addNode(0.f, 80.f);
        graph.addNode(100.f, 0.f);
        graph.addUndirectedEdge(0, 1, 6);
        graph.addUndirectedEdge(1, 3, 30);
        graph.addUndirectedEdge(0, 2, 8);
        graph.addUndirectedEdge(2, 3, 13);
        const auto astarResult = astar.search(graph, 0, 3, SearchStrategy::AStar);
        const auto dijkstra = astar.search(graph, 0, 3, SearchStrategy::Dijkstra);
        const auto greedy = astar.search(graph, 0, 3, SearchStrategy::Greedy);
        const auto weighted = astar.search(graph, 0, 3, SearchStrategy::WeightedAStar);
        require(astarResult.cost == 21, "A* should find the cheaper route");
        require(dijkstra.cost == 21, "Dijkstra should find the cheaper route");
        require(greedy.cost == 36, "Greedy follows the node that looks closest and pays for it");
        require(weighted.found, "Weighted A* should still find a path");
        for (const auto& step : dijkstra.steps) {
            require(step.score.h == 0, "Dijkstra never uses the heuristic");
        }
        require(std::string(strategyName(SearchStrategy::Greedy)) == "Greedy",
                "strategies should have readable names");
        require(astar.search(graph, 0, 3).cost == 21, "the default strategy is A*");
    }

    std::cout << "All AStarLab tests passed.\n";
    return 0;
}
