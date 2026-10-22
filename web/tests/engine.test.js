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

console.log("AStarLab web engine tests passed.");
