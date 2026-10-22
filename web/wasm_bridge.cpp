#include "AStar.hpp"
#include "Graph.hpp"

#include <cstddef>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define ASTARLAB_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define ASTARLAB_EXPORT
#endif

namespace {
Graph graph;
SearchResult result;

const SearchStep& stepAt(int index) {
    return result.steps.at(static_cast<std::size_t>(index));
}
} // namespace

extern "C" {
ASTARLAB_EXPORT void reset_graph() { graph.clear(); }
ASTARLAB_EXPORT int add_node(float x, float y) { return graph.addNode(x, y); }
ASTARLAB_EXPORT void add_edge(int from, int to, int cost, int enabled) {
    graph.addUndirectedEdge(from, to, cost);
    graph.edges().back().enabled = enabled != 0;
}
ASTARLAB_EXPORT void run_search(int start, int goal) { result = AStar{}.search(graph, start, goal); }
ASTARLAB_EXPORT int result_found() { return result.found ? 1 : 0; }
ASTARLAB_EXPORT int result_cost() { return result.cost; }
ASTARLAB_EXPORT int result_expanded_count() { return static_cast<int>(result.expanded.size()); }
ASTARLAB_EXPORT int result_path_count() { return static_cast<int>(result.path.size()); }
ASTARLAB_EXPORT int result_path_node(int index) {
    return result.path.at(static_cast<std::size_t>(index));
}
ASTARLAB_EXPORT int result_step_count() { return static_cast<int>(result.steps.size()); }
ASTARLAB_EXPORT int step_type(int index) { return static_cast<int>(stepAt(index).type); }
ASTARLAB_EXPORT int step_line(int index) { return stepAt(index).pseudocodeLine; }
ASTARLAB_EXPORT int step_current(int index) { return stepAt(index).currentNode; }
ASTARLAB_EXPORT int step_neighbor(int index) { return stepAt(index).neighborNode; }
ASTARLAB_EXPORT int step_edge_cost(int index) { return stepAt(index).edgeCost; }
ASTARLAB_EXPORT int step_tentative_g(int index) { return stepAt(index).tentativeG; }
ASTARLAB_EXPORT int step_previous_g(int index) { return stepAt(index).previousG; }
ASTARLAB_EXPORT int step_improved(int index) { return stepAt(index).improved ? 1 : 0; }
ASTARLAB_EXPORT int step_score_node(int index) { return stepAt(index).score.nodeId; }
ASTARLAB_EXPORT int step_score_g(int index) { return stepAt(index).score.g; }
ASTARLAB_EXPORT int step_score_h(int index) { return stepAt(index).score.h; }
ASTARLAB_EXPORT int step_score_f(int index) { return stepAt(index).score.f; }
}
