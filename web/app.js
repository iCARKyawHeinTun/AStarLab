(function () {
  "use strict";

  const W = 910, H = 570, NODE_RADIUS = 15;
  const basePositions = [
    [60,90],[190,50],[335,85],[480,45],[625,85],[790,55],
    [115,205],[260,195],[410,210],[555,190],[710,225],[850,175],
    [55,350],[205,340],[350,365],[500,335],[655,355],[825,325],
    [135,500],[300,485],[465,510],[640,490],[850,490],
  ];
  const initialObstacles = () => ([
    { type: "circle", x: 380, y: 270, radius: 58 },
    { type: "circle", x: 690, y: 155, radius: 45 },
    { type: "rectangle", x: 545, y: 370, width: 125, height: 82 },
  ]);
  const T = AStarEngine.StepType;
  const STRATEGY_TEXT = {
    astar: { pop: "popLowestF", key: "f = g + h", label: (s) => `${s.g}+${s.h}=${s.f}` },
    dijkstra: { pop: "popLowestG", key: "g", label: (s) => `g=${s.g}` },
    greedy: { pop: "popLowestH", key: "h", label: (s) => `h=${s.h}` },
    weighted: { pop: "popLowestF", key: "f = g + 2h", label: (s) => `${s.g}+2·${s.h}=${s.f}` },
  };
  const SPEED_MS = { line: [1100, 650, 380, 180, 60], node: [1600, 1000, 600, 300, 120] };

  const $ = (id) => document.getElementById(id);
  const elements = Object.fromEntries([
    "run-button", "step-button", "back-button", "restart-button", "finish-button", "random-button", "reset-button",
    "obstacle-tool", "strategy-select", "detail-select", "speed-input", "predict-input",
    "learn-button", "theme-button", "code-button", "challenge-button", "challenge-count", "challenge-panel", "close-challenges", "challenge-list",
    "status", "workspace", "algorithm-panel", "graph-canvas", "engine-badge", "strategy-name", "pop-name", "priority-name",
    "pseudocode", "active-nodes", "g-value", "h-value", "f-value", "calc-line", "open-set", "closed-count",
    "compare-box", "compare-table", "learn-dialog", "close-learn",
  ].map((id) => [id, $(id)]));
  const canvas = elements["graph-canvas"];
  const ctx = canvas.getContext("2d");

  const store = {
    get(key, fallback) { try { const v = localStorage.getItem(key); return v === null ? fallback : JSON.parse(v); } catch (e) { return fallback; } },
    set(key, value) { try { localStorage.setItem(key, JSON.stringify(value)); } catch (e) { /* ignore */ } },
  };

  const state = {
    nodes: [], edges: [], obstacles: initialObstacles(), start: 0, goal: 22,
    engine: null, jsEngine: new AStarEngine.JavaScriptEngine(), strategy: "astar",
    result: null, pos: -1,
    // Derived by replaying steps 0..pos:
    open: new Map(), closed: new Set(), current: null, neighbor: -1, activeLine: -1, lastStep: null,
    playing: false, timer: null, detail: "line", speed: 3,
    predict: false, awaiting: false, answered: false, tries: 0, resumeAfterAnswer: false, predictScore: { right: 0, total: 0 },
    dark: true, codeVisible: true, drag: null, movedOnce: false,
    challengesDone: store.get("astarlab-challenges", {}),
  };

  const css = (name) => getComputedStyle(document.documentElement).getPropertyValue(name).trim();
  const distance = (a, b) => Math.hypot(a.x - b.x, a.y - b.y);
  const inf = (v) => (v < 0 ? "∞" : String(v));

  // ------------------------------------------------------------------ graph
  function buildRoadmap(randomized = false) {
    state.nodes = basePositions.map(([x, y], id) => ({
      id,
      x: randomized ? Math.max(35, Math.min(W - 35, x + Math.random() * 64 - 32)) : x,
      y: randomized ? Math.max(35, Math.min(H - 35, y + Math.random() * 64 - 32)) : y,
    }));
    state.edges = [];
    for (let first = 0; first < basePositions.length; first++) {
      for (let second = first + 1; second < basePositions.length; second++) {
        if (Math.hypot(basePositions[first][0] - basePositions[second][0],
          basePositions[first][1] - basePositions[second][1]) > 178) continue;
        const cost = Math.ceil(distance(state.nodes[first], state.nodes[second]) / 10) +
          (randomized ? Math.floor(Math.random() * 9) : 0);
        state.edges.push({ from: first, to: second, cost, enabled: true });
      }
    }
    updateEdgeAvailability();
  }

  function segmentCircle(a, b, obstacle) {
    const dx = b.x - a.x, dy = b.y - a.y;
    const length2 = dx * dx + dy * dy;
    const t = length2 === 0 ? 0 : Math.max(0, Math.min(1,
      ((obstacle.x - a.x) * dx + (obstacle.y - a.y) * dy) / length2));
    return Math.hypot(a.x + dx * t - obstacle.x, a.y + dy * t - obstacle.y) <= obstacle.radius + 3;
  }

  function segmentRectangle(a, b, rectangle) {
    const left = rectangle.x - 3, right = rectangle.x + rectangle.width + 3;
    const top = rectangle.y - 3, bottom = rectangle.y + rectangle.height + 3;
    let min = 0, max = 1;
    const clip = (p, q) => {
      if (p === 0) return q >= 0;
      const ratio = q / p;
      if (p < 0) { if (ratio > max) return false; min = Math.max(min, ratio); }
      else { if (ratio < min) return false; max = Math.min(max, ratio); }
      return true;
    };
    const dx = b.x - a.x, dy = b.y - a.y;
    return clip(-dx, a.x - left) && clip(dx, right - a.x) &&
      clip(-dy, a.y - top) && clip(dy, bottom - a.y);
  }

  function updateEdgeAvailability() {
    state.edges.forEach((edge) => {
      const a = state.nodes[edge.from], b = state.nodes[edge.to];
      edge.enabled = !state.obstacles.some((obstacle) => obstacle.type === "circle"
        ? segmentCircle(a, b, obstacle) : segmentRectangle(a, b, obstacle));
    });
  }

  // ------------------------------------------------------------------ search playback
  /** Every strategy runs on the shared C++ core when it loaded (older builds only had A*); otherwise the JavaScript engine. */
  const engineFor = (strategy) => (state.engine && (strategy === "astar" || state.engine.supportsStrategies) ? state.engine : state.jsEngine);

  function ensureResult() {
    if (!state.result) state.result = engineFor(state.strategy).search(state.nodes, state.edges, state.start, state.goal, state.strategy);
    return state.result;
  }

  function invalidate(message) {
    stop();
    state.result = null;
    state.pos = -1;
    state.awaiting = false; state.answered = false; state.tries = 0;
    derive();
    elements["compare-box"].hidden = true;
    setStatus(message);
    updatePanel(); render(); evaluateChallenges();
  }

  /** Rebuild everything shown on screen by replaying steps 0..pos. Cheap: a run has a few hundred steps. */
  function derive() {
    state.open = new Map(); state.closed = new Set();
    state.current = null; state.neighbor = -1; state.activeLine = -1; state.lastStep = null;
    if (!state.result) return;
    let seq = 0;
    for (let i = 0; i <= state.pos; i++) {
      const step = state.result.steps[i];
      state.activeLine = step.pseudocodeLine;
      state.neighbor = step.neighborNode;
      state.lastStep = step;
      if (step.type === T.Initialize) state.open.set(step.currentNode, { ...step.score, seq: seq++ });
      if (step.type === T.SelectCurrent) {
        state.open.delete(step.currentNode);
        state.closed.add(step.currentNode);
      }
      if ([T.Initialize, T.SelectCurrent, T.CheckGoal, T.GoalReached].includes(step.type)) state.current = step.score;
      if (step.type === T.UpdateNeighbor) state.open.set(step.neighborNode, { ...step.score, seq: seq++ });
      if ([T.BeginLoop, T.GoalReached, T.SearchExhausted].includes(step.type)) state.neighbor = -1;
    }
  }

  const atEnd = () => state.result && state.pos >= state.result.steps.length - 1;
  const nextStep = () => (state.result ? state.result.steps[state.pos + 1] : null);

  /** One press of "Step". Returns false if it stopped to ask for a prediction or reached the end. */
  function stepForward() {
    ensureResult();
    if (atEnd()) return false;
    for (;;) {
      const nx = nextStep();
      if (!nx) break;
      if (nx.type === T.SelectCurrent && state.predict && !state.answered && state.open.size > 0) { ask(); return false; }
      state.pos++;
      if (nx.type === T.SelectCurrent) state.answered = false;
      derive();
      if (state.detail === "line" || [T.SelectCurrent, T.GoalReached, T.SearchExhausted].includes(nx.type)) break;
    }
    present();
    return !atEnd();
  }

  function stepBack() {
    if (!state.result || state.pos < 0) return;
    stop();
    state.awaiting = false; state.answered = false;
    if (state.detail === "line") state.pos--;
    else {
      let i = state.pos - 1;
      while (i >= 0 && state.result.steps[i].type !== T.SelectCurrent) i--;
      state.pos = i;
    }
    derive(); present();
  }

  function finish() {
    ensureResult(); stop();
    state.awaiting = false;
    state.pos = state.result.steps.length - 1;
    derive(); present();
  }

  function restart() { ensureResult(); stop(); state.pos = -1; state.awaiting = false; state.answered = false; derive(); setStatus("Back at the start. Press Step or Play."); updatePanel(); render(); }

  function play() {
    ensureResult();
    if (atEnd()) { state.pos = -1; derive(); }
    state.playing = true;
    elements["run-button"].textContent = "⏸ Pause";
    elements["run-button"].classList.add("cancel");
    const tick = () => {
      if (!state.playing) return;
      const more = stepForward();
      if (state.awaiting) { state.resumeAfterAnswer = true; stop(); return; }
      if (!more) { stop(); return; }
      state.timer = setTimeout(tick, SPEED_MS[state.detail][state.speed - 1]);
    };
    tick();
  }

  function stop() {
    state.playing = false;
    clearTimeout(state.timer);
    elements["run-button"].textContent = "▶ Play";
    elements["run-button"].classList.remove("cancel");
  }

  // ------------------------------------------------------------------ predict mode
  function sortedOpen() {
    return [...state.open.entries()].sort((a, b) => a[1].f - b[1].f || a[1].seq - b[1].seq);
  }

  function ask() {
    state.awaiting = true;
    setStatus(`🤔 Your turn: which open node (dashed ring) will ${STRATEGY_TEXT[state.strategy].pop}() take out next? Click it on the graph.`);
    updatePanel(); render();
  }

  function answer(nodeId) {
    const open = sortedOpen();
    const best = open[0][1].f;
    const chosen = state.open.get(nodeId);
    const key = STRATEGY_TEXT[state.strategy].key;
    if (!chosen) { setStatus(`Node ${nodeId} isn't in the open set. Only dashed nodes can be chosen next.`); return; }
    const nx = nextStep();
    if (chosen.f === best) {
      state.predictScore.total++;
      if (state.tries === 0) state.predictScore.right++;
      const tie = nodeId !== nx.currentNode ? ` It's a tie on ${key} = ${best}; the engine takes node ${nx.currentNode} because it was added to the open set earlier.` : "";
      finishAnswer(`✓ Right: node ${nodeId} has the lowest ${key} (${best}).${tie}`);
      return;
    }
    state.tries++;
    if (state.tries >= 2) {
      state.predictScore.total++;
      finishAnswer(`Not node ${nodeId} (${key} = ${chosen.f}). The answer is node ${nx.currentNode}, with the lowest ${key} = ${best}.`);
      return;
    }
    setStatus(`Not node ${nodeId}: its ${key} = ${chosen.f}. Compare the ${key} values of all the dashed nodes. Which is smallest? Try again.`);
  }

  function finishAnswer(message) {
    state.awaiting = false; state.answered = true; state.tries = 0;
    stepForward();
    setStatus(`${message}  ·  Predictions: ${state.predictScore.right}/${state.predictScore.total} first try`);
    if (state.resumeAfterAnswer) { state.resumeAfterAnswer = false; setTimeout(() => { if (!state.playing && !atEnd()) play(); }, 700); }
  }

  // ------------------------------------------------------------------ text
  function describeStep(step) {
    switch (step.type) {
      case T.Initialize: return `Start: put node ${step.currentNode} in the open set with g = 0.`;
      case T.BeginLoop: return "The open set isn't empty, so keep searching.";
      case T.SelectCurrent: return `Take out node ${step.currentNode}: it has the lowest ${STRATEGY_TEXT[state.strategy].key} in the open set (${step.score.f}). Close it.`;
      case T.CheckGoal: return `Is node ${step.currentNode} the goal? ${step.improved ? "Yes!" : "No."}`;
      case T.InspectNeighbor: return `Look at neighbor ${step.neighborNode} (edge cost ${step.edgeCost}).`;
      case T.CalculateTentative: return `A route to ${step.neighborNode} through ${step.currentNode} would cost g = ${step.score.g} + ${step.edgeCost} = ${step.tentativeG}.`;
      case T.CompareCost: return step.improved
        ? `${step.tentativeG} < ${inf(step.previousG)}${step.previousG < 0 ? " (never reached before)" : ""}: this route is better.`
        : `${step.tentativeG} is not cheaper than the known ${step.previousG}. Keep the old route.`;
      case T.UpdateNeighbor: return `Update node ${step.neighborNode}: g = ${step.score.g}, h = ${step.score.h}, ${STRATEGY_TEXT[state.strategy].key} = ${step.score.f}. Parent = ${step.currentNode}.`;
      case T.GoalReached: return "Goal reached! Follow the parent links back to the start to get the path.";
      default: return "The open set is empty: there is no path to the goal.";
    }
  }

  function present() {
    if (atEnd()) {
      const r = state.result;
      setStatus(r.found
        ? `Done. ${AStarEngine.Strategies[state.strategy].name} found a path of cost ${r.cost} and closed ${state.closed.size} nodes.${state.predict && state.predictScore.total ? `  Predictions: ${state.predictScore.right}/${state.predictScore.total} first try.` : ""}`
        : "Done. The obstacles disconnected the graph, so there is no path.");
      showComparison();
    } else if (state.lastStep) setStatus(describeStep(state.lastStep));
    updatePanel(); render();
  }

  function setStatus(message) { elements.status.textContent = message; }

  function updatePanel() {
    const st = STRATEGY_TEXT[state.strategy];
    elements["strategy-name"].textContent = AStarEngine.Strategies[state.strategy].name;
    elements["pop-name"].textContent = st.pop;
    elements["priority-name"].textContent = st.key;
    elements["engine-badge"].textContent = engineFor(state.strategy).kind;
    elements.pseudocode.querySelectorAll("li").forEach((item) => {
      item.classList.toggle("active", Number(item.dataset.line) === state.activeLine);
    });
    if (!state.current) {
      elements["active-nodes"].textContent = "Press Step to begin";
      ["g-value", "h-value", "f-value"].forEach((id) => { elements[id].textContent = "—"; });
    } else {
      elements["active-nodes"].textContent = `Current node ${state.current.nodeId}` + (state.neighbor >= 0 ? ` · neighbor ${state.neighbor}` : "");
      elements["g-value"].textContent = state.current.g;
      elements["h-value"].textContent = state.current.h;
      elements["f-value"].textContent = state.current.f;
    }
    const ls = state.lastStep;
    elements["calc-line"].textContent = ls && [T.CalculateTentative, T.CompareCost, T.UpdateNeighbor].includes(ls.type) ? describeStep(ls) : "";
    // Open set. While predicting, list it by node number so the answer isn't given away.
    const open = state.awaiting ? [...state.open.entries()].sort((a, b) => a[0] - b[0]) : sortedOpen();
    elements["open-set"].replaceChildren(...(open.length ? open.map(([id, s], k) => {
      const chip = document.createElement("span");
      chip.className = "chip" + (!state.awaiting && k === 0 ? " next" : "");
      chip.textContent = `${id} · ${s.f}`;
      chip.title = `node ${id}: g=${s.g}, h=${s.h}, ${st.key}=${s.f}`;
      return chip;
    }) : [Object.assign(document.createElement("span"), { className: "muted-note", textContent: "empty" })]));
    elements["closed-count"].textContent = state.closed.size;
    elements["back-button"].disabled = !state.result || state.pos < 0;
    elements["restart-button"].disabled = !state.result || state.pos < 0;
    elements["step-button"].disabled = state.awaiting || (state.result && atEnd());
    elements["finish-button"].disabled = state.result && atEnd();
  }

  function showComparison() {
    const rows = Object.entries(AStarEngine.Strategies).map(([key, s]) => {
      const r = state.jsEngine.search(state.nodes, state.edges, state.start, state.goal, key);
      return { key, name: s.name, found: r.found, cost: r.cost, expanded: r.expanded.length };
    });
    const best = Math.min(...rows.filter((r) => r.found).map((r) => r.cost));
    const table = elements["compare-table"];
    table.replaceChildren();
    const head = table.insertRow();
    ["Strategy", "Cost", "Closed"].forEach((t) => { const th = document.createElement("th"); th.textContent = t; head.appendChild(th); });
    for (const r of rows) {
      const tr = table.insertRow();
      if (r.key === state.strategy) tr.className = "current";
      [r.name, r.found ? `${r.cost}${r.cost === best ? " ✓" : ""}` : "no path", String(r.expanded)].forEach((t) => { tr.insertCell().textContent = t; });
    }
    elements["compare-box"].hidden = false;
  }

  // ------------------------------------------------------------------ challenges
  function evaluate(strategy, edges) { return state.jsEngine.search(state.nodes, edges || state.edges, state.start, state.goal, strategy); }
  const CHALLENGES = [
    { id: "detour", title: "Detour", text: "Make the best path cost at least 20 more than it would with no obstacles at all.",
      hint: "Which edges does the best path use? Block one that has no cheap alternative.",
      check: () => {
        const free = evaluate("astar", state.edges.map((e) => ({ ...e, enabled: true })));
        const now = evaluate("astar");
        return { ok: now.found && now.cost >= free.cost + 20, note: now.found ? `cost ${now.cost}, needs ≥ ${free.cost + 20}` : "no path at all (that's a different challenge!)" };
      } },
    { id: "one", title: "One obstacle is enough", text: "With exactly ONE obstacle on the board, make the goal unreachable.",
      hint: "Count the edges that touch the goal. How many would you need to block?",
      check: () => {
        const r = evaluate("astar");
        return { ok: state.obstacles.length === 1 && !r.found, note: `${state.obstacles.length} obstacle${state.obstacles.length === 1 ? "" : "s"}, goal ${r.found ? "reachable" : "unreachable"}` };
      } },
    { id: "greedy", title: "Greedy's mistake", text: "Make Greedy best-first pay at least 15 more than A* for its path.",
      hint: "Greedy only looks at h, the straight-line distance. Where could a route look short but be expensive?",
      check: () => {
        const a = evaluate("astar"), g = evaluate("greedy");
        return { ok: a.found && g.found && g.cost >= a.cost + 15, note: a.found ? `Greedy ${g.cost} vs A* ${a.cost}` : "no path" };
      } },
    { id: "pays", title: "The heuristic pays off", text: "Make Dijkstra close at least twice as many nodes as A*.",
      hint: "Dijkstra spreads out in every direction; A* leans toward the goal. Try moving the start and the goal (Mouse → Set start / Set goal).",
      check: () => {
        const a = evaluate("astar"), d = evaluate("dijkstra");
        return { ok: a.found && d.expanded.length >= 2 * a.expanded.length, note: a.found ? `Dijkstra ${d.expanded.length} vs A* ${a.expanded.length}` : "no path" };
      } },
    { id: "sweat", title: "Make A* sweat", text: "Keep a path, but make A* close at least 21 nodes before it reaches the goal.",
      hint: "A* trusts the straight-line estimate. What if the straight line is a trap?",
      check: () => {
        const a = evaluate("astar");
        return { ok: a.found && a.expanded.length >= 21, note: a.found ? `A* closed ${a.expanded.length}` : "no path" };
      } },
  ];

  function evaluateChallenges() {
    const list = elements["challenge-list"];
    list.replaceChildren(...CHALLENGES.map((c) => {
      const res = c.check();
      if (res.ok && !state.challengesDone[c.id]) { state.challengesDone[c.id] = true; store.set("astarlab-challenges", state.challengesDone); }
      const done = state.challengesDone[c.id];
      const li = document.createElement("li");
      li.className = (res.ok ? "ok" : "") + (done ? " done" : "");
      li.innerHTML = `<strong>${done ? "✓ " : ""}${c.title}</strong><span>${c.text}</span><small>Right now: ${res.note}${res.ok ? " · solved!" : ""}</small>`;
      const hint = document.createElement("button");
      hint.className = "link"; hint.textContent = "hint";
      hint.addEventListener("click", () => { hint.replaceWith(Object.assign(document.createElement("small"), { className: "hint", textContent: "💡 " + c.hint })); });
      if (!done) li.appendChild(hint);
      return li;
    }));
    const n = CHALLENGES.filter((c) => state.challengesDone[c.id]).length;
    elements["challenge-count"].textContent = `${n}/${CHALLENGES.length}`;
  }

  // ------------------------------------------------------------------ pointer
  function pointFromEvent(event) {
    const rect = canvas.getBoundingClientRect();
    return { x: (event.clientX - rect.left) * W / rect.width, y: (event.clientY - rect.top) * H / rect.height };
  }
  function hitObstacle(point) {
    for (let i = state.obstacles.length - 1; i >= 0; i--) {
      const o = state.obstacles[i];
      const hit = o.type === "circle" ? Math.hypot(point.x - o.x, point.y - o.y) <= o.radius
        : point.x >= o.x && point.x <= o.x + o.width && point.y >= o.y && point.y <= o.y + o.height;
      if (hit) return i;
    }
    return -1;
  }
  function hitNode(point) {
    const r = NODE_RADIUS * scale() * 1.5;
    let best = -1, bestD = Infinity;
    for (const n of state.nodes) { const d = distance(n, point); if (d < r && d < bestD) { best = n.id; bestD = d; } }
    return best;
  }

  canvas.addEventListener("pointerdown", (event) => {
    const point = pointFromEvent(event);
    if (state.awaiting) { const n = hitNode(point); if (n >= 0) answer(n); return; }
    const tool = elements["obstacle-tool"].value;
    if (tool === "start" || tool === "goal") {
      const n = hitNode(point);
      if (n < 0) { setStatus("Click on a node to choose it."); return; }
      if ((tool === "start" && n === state.goal) || (tool === "goal" && n === state.start)) { setStatus("Start and goal must be different nodes."); return; }
      state[tool] = n;
      invalidate(`${tool === "start" ? "Start" : "Goal"} is now node ${n}. Press Play or Step.`);
      return;
    }
    const hit = hitObstacle(point);
    if (event.button === 2 || tool === "delete") {
      if (hit >= 0) { state.obstacles.splice(hit, 1); updateEdgeAvailability(); invalidate("Obstacle deleted. The search starts over."); }
      return;
    }
    if (tool === "circle" || tool === "rectangle") {
      state.obstacles.push(tool === "circle"
        ? { type: "circle", x: point.x, y: point.y, radius: 42 }
        : { type: "rectangle", x: point.x - 52, y: point.y - 36, width: 104, height: 72 });
      elements["obstacle-tool"].value = "move";
      updateEdgeAvailability(); invalidate(`${tool === "circle" ? "Circle" : "Rectangle"} added. Drag it around, then run the search.`);
      return;
    }
    if (hit >= 0) {
      const o = state.obstacles[hit];
      state.drag = { index: hit, dx: point.x - o.x, dy: point.y - o.y };
      canvas.setPointerCapture(event.pointerId);
    }
  });
  canvas.addEventListener("pointermove", (event) => {
    if (!state.drag) {
      const p = pointFromEvent(event);
      canvas.style.cursor = state.awaiting ? (hitNode(p) >= 0 ? "pointer" : "default")
        : ["start", "goal"].includes(elements["obstacle-tool"].value) ? (hitNode(p) >= 0 ? "pointer" : "default")
          : hitObstacle(p) >= 0 ? "grab" : "default";
      return;
    }
    const point = pointFromEvent(event), obstacle = state.obstacles[state.drag.index];
    obstacle.x = Math.max(0, Math.min(W - (obstacle.width || 0), point.x - state.drag.dx));
    obstacle.y = Math.max(0, Math.min(H - (obstacle.height || 0), point.y - state.drag.dy));
    state.movedOnce = true;
    updateEdgeAvailability(); invalidate("The graph changed. Red edges are blocked. Run the search again.");
  });
  const stopDrag = () => { state.drag = null; };
  canvas.addEventListener("pointerup", stopDrag);
  canvas.addEventListener("pointercancel", stopDrag);
  canvas.addEventListener("contextmenu", (event) => event.preventDefault());

  // ------------------------------------------------------------------ drawing
  /** On small screens the 910-unit canvas shrinks a lot, so text and nodes are drawn larger to stay readable. */
  function scale() {
    const w = canvas.getBoundingClientRect().width || W;
    return Math.max(1, Math.min(2.2, (W / w) * 0.62));
  }
  function line(a, b, width, color, dash) {
    ctx.beginPath(); ctx.setLineDash(dash || []); ctx.moveTo(a.x, a.y); ctx.lineTo(b.x, b.y);
    ctx.lineWidth = width; ctx.strokeStyle = color; ctx.stroke(); ctx.setLineDash([]);
  }
  function render() {
    const rect = canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const pixelWidth = Math.max(1, Math.round(rect.width * dpr));
    const pixelHeight = Math.max(1, Math.round(rect.height * dpr));
    if (canvas.width !== pixelWidth || canvas.height !== pixelHeight) { canvas.width = pixelWidth; canvas.height = pixelHeight; }
    ctx.setTransform(pixelWidth / W, 0, 0, pixelHeight / H, 0, 0);
    ctx.clearRect(0, 0, W, H);
    ctx.fillStyle = css("--panel"); ctx.fillRect(0, 0, W, H);
    ctx.textAlign = "center"; ctx.textBaseline = "middle";
    const k = scale();
    const R = NODE_RADIUS * Math.min(k, 1.5);
    const finished = atEnd() && state.result && state.result.found;
    const pathSet = new Set(finished ? state.result.path : []);
    const onPath = (a, b) => {
      if (!finished) return false;
      const p = state.result.path;
      for (let i = 1; i < p.length; i++) if ((p[i - 1] === a && p[i] === b) || (p[i - 1] === b && p[i] === a)) return true;
      return false;
    };

    ctx.font = `${Math.round(11 * k)}px Inter, sans-serif`;
    for (const edge of state.edges) {
      const a = state.nodes[edge.from], b = state.nodes[edge.to];
      const cur = state.current ? state.current.nodeId : -1;
      const inspecting = state.neighbor >= 0 && ((edge.from === cur && edge.to === state.neighbor) || (edge.to === cur && edge.from === state.neighbor));
      if (onPath(edge.from, edge.to)) line(a, b, 7, css("--path"));
      else if (inspecting) line(a, b, 5, css("--path"));
      else line(a, b, edge.enabled ? 2 : 1.5, css(edge.enabled ? "--edge" : "--blocked"), edge.enabled ? null : [6, 5]);
      const mx = (a.x + b.x) / 2, my = (a.y + b.y) / 2;
      const tw = 12 * k;
      ctx.fillStyle = css("--panel"); ctx.fillRect(mx - tw, my - 8 * k, tw * 2, 16 * k);
      ctx.fillStyle = css(edge.enabled ? "--muted" : "--blocked"); ctx.fillText(edge.cost, mx, my);
    }

    for (const obstacle of state.obstacles) {
      ctx.fillStyle = css("--obstacle"); ctx.strokeStyle = css("--obstacle-border"); ctx.lineWidth = 3;
      ctx.globalAlpha = 0.9;
      if (obstacle.type === "circle") { ctx.beginPath(); ctx.arc(obstacle.x, obstacle.y, obstacle.radius, 0, Math.PI * 2); ctx.fill(); ctx.stroke(); }
      else { ctx.beginPath(); ctx.roundRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height, 5); ctx.fill(); ctx.stroke(); }
      ctx.globalAlpha = 1;
      if (!state.movedOnce) {
        ctx.fillStyle = css("--text"); ctx.font = `600 ${Math.round(12 * k)}px Inter, sans-serif`;
        ctx.fillText("drag me", obstacle.type === "circle" ? obstacle.x : obstacle.x + obstacle.width / 2,
          obstacle.type === "circle" ? obstacle.y : obstacle.y + obstacle.height / 2);
      }
    }

    const label = STRATEGY_TEXT[state.strategy].label;
    for (const node of state.nodes) {
      const isStart = node.id === state.start, isGoal = node.id === state.goal;
      const open = state.open.get(node.id);
      const closed = state.closed.has(node.id);
      let fill = css("--node");
      if (closed) fill = css("--expanded");
      if (pathSet.has(node.id)) fill = css("--path");
      if (isStart) fill = css("--start");
      if (isGoal) fill = css("--goal");
      const current = state.current && state.current.nodeId === node.id && !state.awaiting;
      const neighbor = state.neighbor === node.id;
      if (open && state.awaiting) {
        ctx.beginPath(); ctx.arc(node.x, node.y, R + 9, 0, Math.PI * 2);
        ctx.fillStyle = css("--open-glow"); ctx.fill();
      }
      ctx.beginPath(); ctx.arc(node.x, node.y, R, 0, Math.PI * 2);
      ctx.fillStyle = fill; ctx.fill();
      if (open && !isStart && !isGoal) {
        ctx.setLineDash([4, 3]); ctx.lineWidth = 3; ctx.strokeStyle = css("--open"); ctx.stroke(); ctx.setLineDash([]);
      } else {
        ctx.lineWidth = current || neighbor ? 4 : 2;
        ctx.strokeStyle = current ? css("--text") : neighbor ? css("--path") : css("--edge"); ctx.stroke();
      }
      if (current || neighbor) {
        ctx.beginPath(); ctx.arc(node.x, node.y, R + 4, 0, Math.PI * 2);
        ctx.lineWidth = 3; ctx.strokeStyle = current ? css("--text") : css("--path"); ctx.stroke();
      }
      ctx.fillStyle = isStart || isGoal || closed || pathSet.has(node.id) ? "#101318" : css("--text");
      ctx.font = `700 ${Math.round(11 * Math.min(k, 1.5))}px Inter, sans-serif`;
      ctx.fillText(node.id, node.x, node.y);
      const score = open || closedScore(node.id);
      if (score) {
        ctx.font = `${Math.round(10 * k)}px Inter, sans-serif`;
        ctx.fillStyle = open ? css("--open") : css("--muted");
        ctx.fillText(label(score), node.x, node.y + R + 10 * k);
      }
    }
  }
  /** The score a closed node had when it was expanded (for labels). */
  function closedScore(id) {
    if (!state.result || !state.closed.has(id)) return null;
    for (let i = 0; i <= state.pos; i++) {
      const st = state.result.steps[i];
      if (st.type === T.SelectCurrent && st.currentNode === id) return st.score;
    }
    return null;
  }

  // ------------------------------------------------------------------ controls
  elements["run-button"].addEventListener("click", () => (state.playing ? stop() : play()));
  elements["step-button"].addEventListener("click", () => { stop(); stepForward(); });
  elements["back-button"].addEventListener("click", stepBack);
  elements["restart-button"].addEventListener("click", restart);
  elements["finish-button"].addEventListener("click", finish);
  elements["detail-select"].addEventListener("change", (e) => { state.detail = e.target.value; });
  elements["speed-input"].addEventListener("input", (e) => { state.speed = Number(e.target.value); });
  elements["predict-input"].addEventListener("change", (e) => {
    state.predict = e.target.checked;
    state.predictScore = { right: 0, total: 0 };
    if (!state.predict && state.awaiting) { state.awaiting = false; state.answered = true; }
    setStatus(state.predict ? "🤔 Predict mode on: before each node is expanded, you'll click the one you think comes out next." : "Predict mode off.");
    updatePanel(); render();
  });
  elements["strategy-select"].addEventListener("change", (e) => {
    state.strategy = e.target.value;
    invalidate(`Strategy: ${AStarEngine.Strategies[state.strategy].label}. Press Play or Step.`);
  });
  elements["random-button"].addEventListener("click", () => { buildRoadmap(true); invalidate("New node positions and edge costs. Press Play or Step."); });
  elements["reset-button"].addEventListener("click", () => {
    state.obstacles = initialObstacles(); state.start = 0; state.goal = 22; buildRoadmap(false);
    invalidate("Original graph restored.");
  });
  elements["theme-button"].addEventListener("click", () => {
    state.dark = !state.dark; document.documentElement.dataset.theme = state.dark ? "dark" : "light";
    elements["theme-button"].textContent = state.dark ? "☀︎" : "☾";
    elements["theme-button"].setAttribute("aria-label", state.dark ? "Switch to light theme" : "Switch to dark theme");
    render();
  });
  elements["code-button"].addEventListener("click", () => {
    state.codeVisible = !state.codeVisible;
    elements.workspace.classList.toggle("code-hidden", !state.codeVisible);
    elements["algorithm-panel"].hidden = !state.codeVisible;
    elements["code-button"].textContent = state.codeVisible ? "Hide code" : "Show code";
    elements["code-button"].setAttribute("aria-pressed", String(state.codeVisible));
    render();
  });
  const toggleChallenges = (open) => {
    elements["challenge-panel"].hidden = !open;
    elements["challenge-button"].setAttribute("aria-expanded", String(open));
    if (open) { evaluateChallenges(); elements["challenge-panel"].scrollIntoView({ block: "nearest" }); }
  };
  elements["challenge-button"].addEventListener("click", () => toggleChallenges(elements["challenge-panel"].hidden));
  elements["close-challenges"].addEventListener("click", () => toggleChallenges(false));
  elements["learn-button"].addEventListener("click", () => elements["learn-dialog"].showModal());
  elements["close-learn"].addEventListener("click", () => elements["learn-dialog"].close());
  window.addEventListener("resize", render);
  window.addEventListener("keydown", (event) => {
    if (elements["learn-dialog"].open) return;
    // Don't steal keys from buttons, menus, and inputs (Space/Enter would otherwise trigger twice).
    if (event.target.closest && event.target.closest("button, select, input, textarea, a")) return;
    if (event.metaKey || event.ctrlKey || event.altKey) return;
    const key = event.key.toLowerCase();
    if (event.code === "Space") { event.preventDefault(); elements["run-button"].click(); }
    else if (event.key === "ArrowRight") { event.preventDefault(); if (!elements["step-button"].disabled) elements["step-button"].click(); }
    else if (event.key === "ArrowLeft") { event.preventDefault(); stepBack(); }
    else if (event.key === "Home") restart();
    else if (event.key === "End") finish();
    else if (key === "r") elements["reset-button"].click();
    else if (key === "g") elements["random-button"].click();
    else if (key === "t") elements["theme-button"].click();
    else if (key === "p") elements["code-button"].click();
    else if (key === "l") elements["learn-button"].click();
  });

  async function initialize() {
    buildRoadmap(false);
    updatePanel(); render(); evaluateChallenges();
    state.engine = await AStarEngine.createEngine();
    elements["run-button"].disabled = false;
    updatePanel();
  }
  initialize();
})();
