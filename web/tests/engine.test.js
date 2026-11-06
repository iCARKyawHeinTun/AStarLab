"use strict";

const assert = require("node:assert/strict");
const { JavaScriptEngine, StepType } = require("../astar-engine.js");

const engine = new JavaScriptEngine();
const nodes = [{ id: 0, x: 0, y: 0 }, { id: 1, x: 40, y: 20 }, { id: 2, x: 100, y: 0 }];

{
  const edges = [
    { from: 0, to: 2, cost: 12, enabled: true },
    { from: 0, to: 1, cost: 4, enabled: true },
    { from: 1, to: 2, cost: 5, enabled: true },
  ];
  const result = engine.search(nodes, edges, 0, 2);
  assert.equal(result.found, true);
  assert.equal(result.cost, 9);
  assert.deepEqual(result.path, [0, 1, 2]);
  assert.equal(result.steps[0].type, StepType.Initialize);
  assert.equal(result.steps.at(-1).type, StepType.GoalReached);
  assert.equal(result.steps.at(-1).pseudocodeLine, 5);
  assert.equal(result.steps.find(step => step.type === StepType.UpdateNeighbor).pseudocodeLine, 9);
}

{
  const edges = [
    { from: 0, to: 1, cost: 5, enabled: true },
    { from: 1, to: 2, cost: 5, enabled: false },
  ];
  const result = engine.search(nodes, edges, 0, 2);
  assert.equal(result.found, false);
  assert.equal(result.steps.at(-1).type, StepType.SearchExhausted);
}

// Strategies: the same graph, four different priorities.
{
  // Two routes from 0 to 3: a short-looking one through 1 (expensive), and a cheaper one through 2.
  const nodes4 = [{ id: 0, x: 0, y: 0 }, { id: 1, x: 60, y: 0 }, { id: 2, x: 0, y: 80 }, { id: 3, x: 100, y: 0 }];
  const edges4 = [
    { from: 0, to: 1, cost: 6, enabled: true },
    { from: 1, to: 3, cost: 30, enabled: true },
    { from: 0, to: 2, cost: 8, enabled: true },
    { from: 2, to: 3, cost: 13, enabled: true },
  ];
  const astar = engine.search(nodes4, edges4, 0, 3, "astar");
  const dijkstra = engine.search(nodes4, edges4, 0, 3, "dijkstra");
  const greedy = engine.search(nodes4, edges4, 0, 3, "greedy");
  assert.equal(astar.cost, 21);
  assert.equal(dijkstra.cost, 21);
  assert.equal(greedy.cost, 36, "greedy follows the node that looks closest and pays for it");
  assert.ok(dijkstra.steps.every(step => step.score.h === 0), "Dijkstra never uses h");
  assert.deepEqual(engine.search(nodes, [
    { from: 0, to: 2, cost: 12, enabled: true }, { from: 0, to: 1, cost: 4, enabled: true }, { from: 1, to: 2, cost: 5, enabled: true },
  ], 0, 2, "astar").path, [0, 1, 2], "default strategy is A*");
}

// The real roadmap: A* and Dijkstra always agree on cost; A* never expands more nodes.
{
  const base = [[60,90],[190,50],[335,85],[480,45],[625,85],[790,55],[115,205],[260,195],[410,210],[555,190],[710,225],[850,175],
    [55,350],[205,340],[350,365],[500,335],[655,355],[825,325],[135,500],[300,485],[465,510],[640,490],[850,490]];
  for (let trial = 0; trial < 30; trial++) {
    let seed = trial + 1;
    const rand = () => ((seed = (seed * 16807) % 2147483647) / 2147483647);
    const ns = base.map(([x, y], id) => ({ id, x: x + rand() * 64 - 32, y: y + rand() * 64 - 32 }));
    const es = [];
    for (let i = 0; i < base.length; i++) for (let j = i + 1; j < base.length; j++) {
      if (Math.hypot(base[i][0] - base[j][0], base[i][1] - base[j][1]) > 178) continue;
      es.push({ from: i, to: j, cost: Math.ceil(Math.hypot(ns[i].x - ns[j].x, ns[i].y - ns[j].y) / 10) + Math.floor(rand() * 9), enabled: rand() > 0.15 });
    }
    const a = engine.search(ns, es, 0, 22, "astar");
    const d = engine.search(ns, es, 0, 22, "dijkstra");
    assert.equal(a.found, d.found);
    if (a.found) {
      assert.equal(a.cost, d.cost, `trial ${trial}: A* must be optimal`);
      assert.ok(a.expanded.length <= d.expanded.length, `trial ${trial}: A* expanded more than Dijkstra`);
    }
  }
}

console.log("AStarLab web engine tests passed.");
