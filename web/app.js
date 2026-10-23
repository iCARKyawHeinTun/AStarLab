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

  const elements = Object.fromEntries([
    "run-button","random-button","obstacle-tool","learn-button","reset-button",
    "theme-button","speed-button","code-button","status","workspace","algorithm-panel","graph-canvas","engine-badge",
    "pseudocode","active-nodes","g-value","h-value","f-value","learn-dialog","close-learn",
  ].map(id => [id, document.getElementById(id)]));
  const canvas = elements["graph-canvas"];
  const ctx = canvas.getContext("2d");
  const state = {
    nodes: [], edges: [], obstacles: initialObstacles(), result: null,
    scores: new Map(), expanded: new Set(), visiblePath: new Set(),
    currentScore: null, neighbor: -1, activeLine: -1,
    running: false, slow: true, dark: true, codeVisible: true, runToken: 0,
    drag: null, engine: null,
  };

  const css = name => getComputedStyle(document.documentElement).getPropertyValue(name).trim();
  const distance = (a, b) => Math.hypot(a.x - b.x, a.y - b.y);
  const wait = ms => new Promise(resolve => setTimeout(resolve, ms));

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
    state.edges.forEach(edge => {
      const a = state.nodes[edge.from], b = state.nodes[edge.to];
      edge.enabled = !state.obstacles.some(obstacle => obstacle.type === "circle"
        ? segmentCircle(a, b, obstacle) : segmentRectangle(a, b, obstacle));
    });
  }

  function resetVisualization(message) {
    state.runToken++;
    state.result = null;
    state.scores.clear(); state.expanded.clear(); state.visiblePath.clear();
    state.currentScore = null; state.neighbor = -1; state.activeLine = -1;
    setRunning(false); setStatus(message); updatePanel(); render();
  }

  function setStatus(message) { elements.status.textContent = message; }
  function setRunning(running) {
    state.running = running;
    elements["run-button"].textContent = running ? "Cancel" : "Run";
    elements["run-button"].classList.toggle("cancel", running);
    ["random-button", "obstacle-tool", "learn-button", "reset-button"].forEach(id => {
      elements[id].disabled = running;
    });
  }

  function updatePanel() {
    elements.pseudocode.querySelectorAll("li").forEach(item => {
      item.classList.toggle("active", Number(item.dataset.line) === state.activeLine);
    });
    if (!state.currentScore) {
      elements["active-nodes"].textContent = "Run the search to begin";
      ["g-value", "h-value", "f-value"].forEach(id => { elements[id].textContent = "—"; });
      return;
    }
    elements["active-nodes"].textContent = `Current ${state.currentScore.nodeId}` +
      (state.neighbor >= 0 ? ` · Neighbor ${state.neighbor}` : "");
    elements["g-value"].textContent = state.currentScore.g;
    elements["h-value"].textContent = state.currentScore.h;
    elements["f-value"].textContent = state.currentScore.f;
  }

  function describeStep(step) {
    const T = AStarEngine.StepType;
    switch (step.type) {
      case T.Initialize: return `Start at node ${step.currentNode} and add it to open`;
      case T.BeginLoop: return "The open set is not empty — continue searching";
      case T.SelectCurrent: return `Choose node ${step.currentNode} with f=${step.score.f}`;
      case T.CheckGoal: return `Is node ${step.currentNode} the goal? ${step.improved ? "Yes" : "No"}`;
      case T.InspectNeighbor: return `Inspect neighbor ${step.neighborNode} (edge cost ${step.edgeCost})`;
      case T.CalculateTentative: return `New g for node ${step.neighborNode} = ${step.score.g} + ${step.edgeCost} = ${step.tentativeG}`;
      case T.CompareCost: return step.improved ? "This route is cheaper" : `Keep the existing route (old g=${step.previousG})`;
      case T.UpdateNeighbor: return `Update node ${step.neighborNode}: g=${step.score.g}, h=${step.score.h}, f=${step.score.f}`;
      case T.GoalReached: return "Goal reached — rebuild the path using parents";
      default: return "Open is empty, so no path exists";
    }
  }

  function applyStep(step, present = true) {
    const T = AStarEngine.StepType;
    state.activeLine = step.pseudocodeLine;
    state.neighbor = step.neighborNode;
    if ([T.Initialize, T.SelectCurrent, T.CheckGoal].includes(step.type)) state.currentScore = step.score;
    if (step.type === T.Initialize || step.type === T.SelectCurrent) state.scores.set(step.currentNode, step.score);
    if (step.type === T.SelectCurrent) state.expanded.add(step.currentNode);
    if (step.type === T.UpdateNeighbor) state.scores.set(step.neighborNode, step.score);
    if ([T.BeginLoop, T.GoalReached, T.SearchExhausted].includes(step.type)) state.neighbor = -1;
    if (present) { setStatus(describeStep(step)); updatePanel(); render(); }
  }

  async function runSearch() {
    if (state.running) { resetVisualization("Simulation canceled — obstacles are unlocked"); return; }
    const token = ++state.runToken;
    state.scores.clear(); state.expanded.clear(); state.visiblePath.clear();
    state.currentScore = null; state.neighbor = -1; state.activeLine = -1;
    setRunning(true);
    state.result = state.engine.search(state.nodes, state.edges, 0, 22);
    let stepIndex = 0;
    while (stepIndex < state.result.steps.length) {
      if (token !== state.runToken) return;
      if (state.slow) {
        applyStep(state.result.steps[stepIndex++]);
        await wait(520);
      } else {
        let checkpoint = null;
        do {
          checkpoint = state.result.steps[stepIndex++];
          applyStep(checkpoint, false);
        } while (stepIndex < state.result.steps.length &&
          ![AStarEngine.StepType.SelectCurrent, AStarEngine.StepType.GoalReached,
            AStarEngine.StepType.SearchExhausted].includes(checkpoint.type));
        state.activeLine = -1;
        state.neighbor = -1;
        setStatus(checkpoint.type === AStarEngine.StepType.SelectCurrent
          ? `Exploring node ${checkpoint.currentNode} · f=${checkpoint.score.f}`
          : describeStep(checkpoint));
        updatePanel(); render();
        await wait(220);
      }
    }
    if (state.result.found) {
      setStatus("Goal reached — tracing the lowest-cost path");
      for (const node of state.result.path) {
        if (token !== state.runToken) return;
        state.visiblePath.add(node); render();
        await wait(state.slow ? 360 : 120);
      }
      setStatus(`Best path: cost ${state.result.cost} · explored ${state.expanded.size} nodes`);
    } else setStatus("No path — the obstacles disconnected the graph");
    if (token === state.runToken) setRunning(false);
  }

  function pointFromEvent(event) {
    const rect = canvas.getBoundingClientRect();
    return { x: (event.clientX - rect.left) * W / rect.width,
      y: (event.clientY - rect.top) * H / rect.height };
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
  function graphChanged(message) {
    updateEdgeAvailability(); resetVisualization(message);
  }

  canvas.addEventListener("pointerdown", event => {
    if (state.running) { setStatus("Cancel the simulation before editing obstacles"); return; }
    const point = pointFromEvent(event);
    const tool = elements["obstacle-tool"].value;
    const hit = hitObstacle(point);
    if (event.button === 2 || tool === "delete") {
      if (hit >= 0) { state.obstacles.splice(hit, 1); graphChanged("Obstacle deleted — run the search again"); }
      return;
    }
    if (tool === "circle") {
      state.obstacles.push({ type: "circle", x: point.x, y: point.y, radius: 42 });
      elements["obstacle-tool"].value = "move"; graphChanged("Circle added — drag it or run the search"); return;
    }
    if (tool === "rectangle") {
      state.obstacles.push({ type: "rectangle", x: point.x - 52, y: point.y - 36, width: 104, height: 72 });
      elements["obstacle-tool"].value = "move"; graphChanged("Rectangle added — drag it or run the search"); return;
    }
    if (hit >= 0) {
      const o = state.obstacles[hit];
      state.drag = { index: hit, dx: point.x - o.x, dy: point.y - o.y };
      canvas.setPointerCapture(event.pointerId);
    }
  });
  canvas.addEventListener("pointermove", event => {
    if (!state.drag || state.running) return;
    const point = pointFromEvent(event), obstacle = state.obstacles[state.drag.index];
    obstacle.x = Math.max(0, Math.min(W - (obstacle.width || 0), point.x - state.drag.dx));
    obstacle.y = Math.max(0, Math.min(H - (obstacle.height || 0), point.y - state.drag.dy));
    updateEdgeAvailability(); resetVisualization("The graph changed — run the search again");
  });
  const stopDrag = () => { state.drag = null; };
  canvas.addEventListener("pointerup", stopDrag);
  canvas.addEventListener("pointercancel", stopDrag);
  canvas.addEventListener("contextmenu", event => event.preventDefault());

  function roundedRect(x, y, width, height, radius) {
    ctx.beginPath(); ctx.roundRect(x, y, width, height, radius); ctx.fill(); ctx.stroke();
  }
  function line(a, b, width, color) {
    ctx.beginPath(); ctx.moveTo(a.x, a.y); ctx.lineTo(b.x, b.y);
    ctx.lineWidth = width; ctx.strokeStyle = color; ctx.stroke();
  }
  function render() {
    const rect = canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const pixelWidth = Math.max(1, Math.round(rect.width * dpr));
    const pixelHeight = Math.max(1, Math.round(rect.height * dpr));
    if (canvas.width !== pixelWidth || canvas.height !== pixelHeight) {
      canvas.width = pixelWidth; canvas.height = pixelHeight;
    }
    ctx.setTransform(pixelWidth / W, 0, 0, pixelHeight / H, 0, 0);
    ctx.clearRect(0, 0, W, H);
    ctx.fillStyle = css("--panel"); ctx.fillRect(0, 0, W, H);
    ctx.textAlign = "center"; ctx.textBaseline = "middle"; ctx.font = "11px Inter, sans-serif";

    for (const edge of state.edges) {
      const a = state.nodes[edge.from], b = state.nodes[edge.to];
      const inspecting = state.currentScore && state.neighbor >= 0 &&
        ((edge.from === state.currentScore.nodeId && edge.to === state.neighbor) ||
         (edge.to === state.currentScore.nodeId && edge.from === state.neighbor));
      line(a, b, inspecting ? 5 : (edge.enabled ? 2 : 1), inspecting ? css("--path") : css(edge.enabled ? "--edge" : "--blocked"));
      const mx = (a.x + b.x) / 2, my = (a.y + b.y) / 2;
      ctx.fillStyle = css("--panel"); ctx.fillRect(mx - 12, my - 8, 24, 16);
      ctx.fillStyle = css(edge.enabled ? "--muted" : "--blocked"); ctx.fillText(edge.cost, mx, my);
    }

    if (state.result && state.visiblePath.size > 1) {
      for (let i = 1; i < state.result.path.length; i++) {
        if (state.visiblePath.has(state.result.path[i - 1]) && state.visiblePath.has(state.result.path[i]))
          line(state.nodes[state.result.path[i - 1]], state.nodes[state.result.path[i]], 6, css("--path"));
      }
    }

    for (const obstacle of state.obstacles) {
      ctx.fillStyle = css("--obstacle"); ctx.strokeStyle = css("--obstacle-border"); ctx.lineWidth = 3;
      if (obstacle.type === "circle") {
        ctx.beginPath(); ctx.arc(obstacle.x, obstacle.y, obstacle.radius, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
      } else roundedRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height, 5);
      ctx.fillStyle = css("--text"); ctx.font = "600 12px Inter, sans-serif";
      ctx.fillText("Move me", obstacle.type === "circle" ? obstacle.x : obstacle.x + obstacle.width / 2,
        obstacle.type === "circle" ? obstacle.y : obstacle.y + obstacle.height / 2);
    }

    for (const node of state.nodes) {
      let fill = css("--node");
      if (state.expanded.has(node.id)) fill = css("--expanded");
      if (state.visiblePath.has(node.id)) fill = css("--path");
      if (node.id === 0) fill = css("--start");
      if (node.id === 22) fill = css("--goal");
      const current = state.currentScore && state.currentScore.nodeId === node.id;
      const neighbor = state.neighbor === node.id;
      ctx.beginPath(); ctx.arc(node.x, node.y, NODE_RADIUS, 0, Math.PI * 2);
      ctx.fillStyle = fill; ctx.fill(); ctx.lineWidth = current || neighbor ? 4 : 2;
      ctx.strokeStyle = current ? css("--text") : neighbor ? css("--path") : css("--edge"); ctx.stroke();
      ctx.fillStyle = css("--text"); ctx.font = "700 11px Inter, sans-serif"; ctx.fillText(node.id, node.x, node.y);
      const nodeScore = state.scores.get(node.id);
      if (nodeScore && node.id !== 0 && node.id !== 22) {
        ctx.font = "10px Inter, sans-serif";
        ctx.fillText(`g${nodeScore.g}  f${nodeScore.f}`, node.x, node.y + 26);
      }
    }
  }

  elements["run-button"].addEventListener("click", runSearch);
  elements["random-button"].addEventListener("click", () => {
    buildRoadmap(true); resetVisualization("New nodes and edge costs generated — run the search");
  });
  elements["reset-button"].addEventListener("click", () => {
    state.obstacles = initialObstacles(); buildRoadmap(false);
    resetVisualization("Ready — move an obstacle or run the search");
  });
  elements["theme-button"].addEventListener("click", () => {
    state.dark = !state.dark; document.documentElement.dataset.theme = state.dark ? "dark" : "light";
    elements["theme-button"].textContent = state.dark ? "Light" : "Dark"; render();
  });
  elements["speed-button"].addEventListener("click", () => {
    state.slow = !state.slow; elements["speed-button"].textContent = state.slow ? "Faster" : "Slower";
    setStatus(state.slow ? "Slower playback — following every pseudocode line"
      : "Faster playback — grouping pseudocode steps");
  });
  elements["code-button"].addEventListener("click", () => {
    state.codeVisible = !state.codeVisible;
    elements.workspace.classList.toggle("code-hidden", !state.codeVisible);
    elements["algorithm-panel"].hidden = !state.codeVisible;
    elements["algorithm-panel"].style.display = state.codeVisible ? "" : "none";
    elements.workspace.style.gridTemplateColumns = state.codeVisible ? "" : "minmax(0, 1fr)";
    elements["code-button"].textContent = state.codeVisible ? "Hide code" : "Show code";
    elements["code-button"].setAttribute("aria-pressed", String(state.codeVisible));
    setStatus(state.codeVisible ? "Synchronized pseudocode visible" : "Synchronized pseudocode hidden");
    render();
  });
  elements["learn-button"].addEventListener("click", () => elements["learn-dialog"].showModal());
  elements["close-learn"].addEventListener("click", () => elements["learn-dialog"].close());
  window.addEventListener("resize", render);
  window.addEventListener("keydown", event => {
    if (elements["learn-dialog"].open) return;
    if (event.code === "Space" || event.code === "Enter") { event.preventDefault(); runSearch(); }
    if (event.key.toLowerCase() === "r" && !state.running) elements["reset-button"].click();
    if (event.key.toLowerCase() === "g" && !state.running) elements["random-button"].click();
    if (event.key.toLowerCase() === "s") elements["speed-button"].click();
    if (event.key.toLowerCase() === "t") elements["theme-button"].click();
    if (event.key.toLowerCase() === "p") elements["code-button"].click();
    if (event.key.toLowerCase() === "l" && !state.running) elements["learn-button"].click();
  });

  async function initialize() {
    buildRoadmap(false); render();
    state.engine = await AStarEngine.createEngine();
    elements["engine-badge"].textContent = state.engine.kind;
    elements["run-button"].disabled = false;
  }
  initialize();
})();
