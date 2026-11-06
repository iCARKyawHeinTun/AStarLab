(function (root) {
  "use strict";

  const StepType = Object.freeze({
    Initialize: 0, BeginLoop: 1, SelectCurrent: 2, CheckGoal: 3,
    InspectNeighbor: 4, CalculateTentative: 5, CompareCost: 6,
    UpdateNeighbor: 7, GoalReached: 8, SearchExhausted: 9,
  });

  const score = (nodeId = 0, g = 0, h = 0, f = 0) => ({ nodeId, g, h, f });
  const heuristic = (a, b) => Math.floor(Math.hypot(a.x - b.x, a.y - b.y) / 10);

  // Search strategies. Each one only changes the number the open set is sorted by.
  // "f" in the returned scores is always that priority, so the UI can show it.
  const Strategies = Object.freeze({
    astar: { name: "A*", label: "A*  (f = g + h)", priority: (g, h) => g + h, usesH: true },
    dijkstra: { name: "Dijkstra", label: "Dijkstra  (f = g, no heuristic)", priority: (g) => g, usesH: false },
    greedy: { name: "Greedy", label: "Greedy best-first  (f = h)", priority: (g, h) => h, usesH: true },
    weighted: { name: "Weighted A*", label: "Weighted A*  (f = g + 2h)", priority: (g, h) => g + 2 * h, usesH: true },
  });

  class JavaScriptEngine {
    constructor() { this.kind = "JavaScript fallback"; }

    search(nodes, edges, start, goal, strategy = "astar") {
      const strat = Strategies[strategy] || Strategies.astar;
      const hOf = id => (strat.usesH ? heuristic(nodes[id], nodes[goal]) : 0);
      const result = { found: false, cost: 0, expanded: [], path: [], steps: [] };
      const neighbors = Array.from({ length: nodes.length }, () => []);
      for (const edge of edges) {
        if (!edge.enabled) continue;
        neighbors[edge.from].push([edge.to, edge.cost]);
        neighbors[edge.to].push([edge.from, edge.cost]);
      }

      const open = [];
      const gScore = new Map([[start, 0]]);
      const cameFrom = new Map();
      const closed = new Set();
      let sequence = 0;
      const startH = hOf(start);
      const startF = strat.priority(0, startH);
      open.push({ nodeId: start, f: startF, sequence: sequence++ });
      result.steps.push(this.step(StepType.Initialize, 1, start, -1, 0, 0, -1, true,
        score(start, 0, startH, startF)));

      while (open.length) {
        result.steps.push(this.step(StepType.BeginLoop, 2));
        open.sort((a, b) => a.f - b.f || a.sequence - b.sequence);
        const current = open.shift().nodeId;
        if (closed.has(current)) continue;
        const h = hOf(current);
        const currentScore = score(current, gScore.get(current), h, strat.priority(gScore.get(current), h));
        closed.add(current);
        result.expanded.push(currentScore);
        result.steps.push(this.step(StepType.SelectCurrent, 3, current, -1, 0, 0, -1, false, currentScore));
        result.steps.push(this.step(StepType.CheckGoal, 4, current, -1, 0, 0, -1, current === goal, currentScore));

        if (current === goal) {
          result.found = true;
          result.cost = gScore.get(current);
          let cursor = goal;
          result.path.push(cursor);
          while (cursor !== start) {
            cursor = cameFrom.get(cursor);
            result.path.push(cursor);
          }
          result.path.reverse();
          result.steps.push(this.step(StepType.GoalReached, 5, current, -1, 0,
            gScore.get(current), -1, true, currentScore));
          return result;
        }

        for (const [neighbor, edgeCost] of neighbors[current]) {
          if (closed.has(neighbor)) continue;
          result.steps.push(this.step(StepType.InspectNeighbor, 6, current, neighbor,
            edgeCost, 0, -1, false, currentScore));
          const tentative = gScore.get(current) + edgeCost;
          const previous = gScore.has(neighbor) ? gScore.get(neighbor) : -1;
          result.steps.push(this.step(StepType.CalculateTentative, 7, current, neighbor,
            edgeCost, tentative, previous, false, currentScore));
          const improved = previous < 0 || tentative < previous;
          result.steps.push(this.step(StepType.CompareCost, 8, current, neighbor,
            edgeCost, tentative, previous, improved, currentScore));
          if (improved) {
            cameFrom.set(neighbor, current);
            gScore.set(neighbor, tentative);
            const estimate = hOf(neighbor);
            const neighborScore = score(neighbor, tentative, estimate, strat.priority(tentative, estimate));
            open.push({ nodeId: neighbor, f: neighborScore.f, sequence: sequence++ });
            result.steps.push(this.step(StepType.UpdateNeighbor, 9, current, neighbor,
              edgeCost, tentative, previous, true, neighborScore));
          }
        }
      }
      result.steps.push(this.step(StepType.SearchExhausted, 2));
      return result;
    }

    step(type, pseudocodeLine, currentNode = -1, neighborNode = -1, edgeCost = 0,
      tentativeG = 0, previousG = -1, improved = false, nodeScore = score()) {
      return { type, pseudocodeLine, currentNode, neighborNode, edgeCost,
        tentativeG, previousG, improved, score: { ...nodeScore } };
    }
  }

  class WasmEngine {
    constructor(module) {
      this.module = module;
      this.kind = "Shared C++ core · WebAssembly";
      this.supportsStrategies = typeof module._run_search_strategy === "function";
    }
    call(name, returnType = "number", args = [], values = []) {
      return this.module.ccall(name, returnType, args, values);
    }
    search(nodes, edges, start, goal, strategy = "astar") {
      this.call("reset_graph", null);
      nodes.forEach(node => this.call("add_node", "number", ["number", "number"], [node.x, node.y]));
      edges.forEach(edge => this.call("add_edge", null,
        ["number", "number", "number", "number"], [edge.from, edge.to, edge.cost, edge.enabled ? 1 : 0]));
      const strategyIndex = ["astar", "dijkstra", "greedy", "weighted"].indexOf(strategy);
      if (this.supportsStrategies) {
        this.call("run_search_strategy", null, ["number", "number", "number"], [start, goal, Math.max(0, strategyIndex)]);
      } else {
        this.call("run_search", null, ["number", "number"], [start, goal]);
      }
      const count = this.call("result_step_count");
      const steps = [];
      for (let index = 0; index < count; index++) {
        const get = name => this.call(name, "number", ["number"], [index]);
        steps.push({
          type: get("step_type"), pseudocodeLine: get("step_line"),
          currentNode: get("step_current"), neighborNode: get("step_neighbor"),
          edgeCost: get("step_edge_cost"), tentativeG: get("step_tentative_g"),
          previousG: get("step_previous_g"), improved: Boolean(get("step_improved")),
          score: { nodeId: get("step_score_node"), g: get("step_score_g"),
            h: get("step_score_h"), f: get("step_score_f") },
        });
      }
      const path = [];
      const pathCount = this.call("result_path_count");
      for (let index = 0; index < pathCount; index++) {
        path.push(this.call("result_path_node", "number", ["number"], [index]));
      }
      return {
        found: Boolean(this.call("result_found")), cost: this.call("result_cost"),
        expandedCount: this.call("result_expanded_count"), path, steps,
      };
    }
  }

  async function createEngine() {
    if (typeof root.createAStarCore === "function") {
      try { return new WasmEngine(await root.createAStarCore()); }
      catch (error) { console.warn("AStarLab C++ core could not load; using fallback.", error); }
    }
    return new JavaScriptEngine();
  }

  const api = { StepType, Strategies, heuristic, JavaScriptEngine, WasmEngine, createEngine };
  root.AStarEngine = api;
  if (typeof module !== "undefined" && module.exports) module.exports = api;
})(typeof globalThis !== "undefined" ? globalThis : window);
