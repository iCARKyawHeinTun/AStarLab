#include "Application.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <sstream>
#include <string_view>
#include <tuple>
#include <unordered_map>

namespace {
sf::Color Background{29, 31, 35};
sf::Color Panel{41, 44, 49};
sf::Color PanelLight{50, 53, 59};
sf::Color Edge{94, 102, 114};
sf::Color BlockedEdge{190, 111, 117};
sf::Color Node{62, 66, 73};
sf::Color Expanded{96, 164, 184};
sf::Color Path{232, 176, 72};
sf::Color Start{72, 178, 128};
sf::Color Goal{221, 105, 101};
sf::Color Obstacle{137, 108, 77};
sf::Color ObstacleBorder{218, 164, 99};
sf::Color Text{239, 241, 244};
sf::Color Muted{174, 181, 191};

void applyTheme(bool dark) {
    if (dark) {
        Background = {29, 31, 35};
        Panel = {41, 44, 49};
        PanelLight = {50, 53, 59};
        Edge = {94, 102, 114};
        BlockedEdge = {190, 111, 117};
        Node = {62, 66, 73};
        Expanded = {96, 164, 184};
        Path = {232, 176, 72};
        Start = {72, 178, 128};
        Goal = {221, 105, 101};
        Obstacle = {137, 108, 77};
        ObstacleBorder = {218, 164, 99};
        Text = {239, 241, 244};
        Muted = {174, 181, 191};
        return;
    }

    Background = {247, 245, 240};
    Panel = {255, 255, 255};
    PanelLight = {242, 239, 232};
    Edge = {184, 194, 205};
    BlockedEdge = {220, 132, 139};
    Node = {255, 255, 255};
    Expanded = {94, 169, 192};
    Path = {239, 173, 50};
    Start = {56, 166, 113};
    Goal = {222, 91, 88};
    Obstacle = {239, 205, 161};
    ObstacleBorder = {194, 127, 55};
    Text = {39, 50, 66};
    Muted = {103, 116, 134};
}

using Glyph = std::array<std::string_view, 7>;

const Glyph& glyphFor(char character) {
    static const std::unordered_map<char, Glyph> glyphs{
        {'A', {"01110", "10001", "10001", "11111", "10001", "10001", "10001"}},
        {'B', {"11110", "10001", "10001", "11110", "10001", "10001", "11110"}},
        {'C', {"01111", "10000", "10000", "10000", "10000", "10000", "01111"}},
        {'D', {"11110", "10001", "10001", "10001", "10001", "10001", "11110"}},
        {'E', {"11111", "10000", "10000", "11110", "10000", "10000", "11111"}},
        {'F', {"11111", "10000", "10000", "11110", "10000", "10000", "10000"}},
        {'G', {"01111", "10000", "10000", "10111", "10001", "10001", "01111"}},
        {'H', {"10001", "10001", "10001", "11111", "10001", "10001", "10001"}},
        {'I', {"11111", "00100", "00100", "00100", "00100", "00100", "11111"}},
        {'J', {"00111", "00010", "00010", "00010", "10010", "10010", "01100"}},
        {'K', {"10001", "10010", "10100", "11000", "10100", "10010", "10001"}},
        {'L', {"10000", "10000", "10000", "10000", "10000", "10000", "11111"}},
        {'M', {"10001", "11011", "10101", "10101", "10001", "10001", "10001"}},
        {'N', {"10001", "11001", "10101", "10011", "10001", "10001", "10001"}},
        {'O', {"01110", "10001", "10001", "10001", "10001", "10001", "01110"}},
        {'P', {"11110", "10001", "10001", "11110", "10000", "10000", "10000"}},
        {'Q', {"01110", "10001", "10001", "10001", "10101", "10010", "01101"}},
        {'R', {"11110", "10001", "10001", "11110", "10100", "10010", "10001"}},
        {'S', {"01111", "10000", "10000", "01110", "00001", "00001", "11110"}},
        {'T', {"11111", "00100", "00100", "00100", "00100", "00100", "00100"}},
        {'U', {"10001", "10001", "10001", "10001", "10001", "10001", "01110"}},
        {'V', {"10001", "10001", "10001", "10001", "10001", "01010", "00100"}},
        {'W', {"10001", "10001", "10001", "10101", "10101", "11011", "10001"}},
        {'X', {"10001", "10001", "01010", "00100", "01010", "10001", "10001"}},
        {'Y', {"10001", "10001", "01010", "00100", "00100", "00100", "00100"}},
        {'Z', {"11111", "00001", "00010", "00100", "01000", "10000", "11111"}},
        {'0', {"01110", "10001", "10011", "10101", "11001", "10001", "01110"}},
        {'1', {"00100", "01100", "00100", "00100", "00100", "00100", "01110"}},
        {'2', {"01110", "10001", "00001", "00010", "00100", "01000", "11111"}},
        {'3', {"11110", "00001", "00001", "01110", "00001", "00001", "11110"}},
        {'4', {"00010", "00110", "01010", "10010", "11111", "00010", "00010"}},
        {'5', {"11111", "10000", "10000", "11110", "00001", "00001", "11110"}},
        {'6', {"01110", "10000", "10000", "11110", "10001", "10001", "01110"}},
        {'7', {"11111", "00001", "00010", "00100", "01000", "01000", "01000"}},
        {'8', {"01110", "10001", "10001", "01110", "10001", "10001", "01110"}},
        {'9', {"01110", "10001", "10001", "01111", "00001", "00001", "01110"}},
        {'*', {"00000", "10101", "01110", "11111", "01110", "10101", "00000"}},
        {'+', {"00000", "00100", "00100", "11111", "00100", "00100", "00000"}},
        {'=', {"00000", "00000", "11111", "00000", "11111", "00000", "00000"}},
        {'-', {"00000", "00000", "00000", "11111", "00000", "00000", "00000"}},
        {':', {"00000", "00100", "00100", "00000", "00100", "00100", "00000"}},
        {'.', {"00000", "00000", "00000", "00000", "00000", "00100", "00100"}},
        {'/', {"00001", "00010", "00010", "00100", "01000", "01000", "10000"}},
        {'?', {"01110", "10001", "00010", "00100", "00100", "00000", "00100"}},
    };
    static const Glyph blank{"00000", "00000", "00000", "00000", "00000", "00000", "00000"};
    static const Glyph unknown = glyphs.at('?');
    if (character == ' ') return blank;
    const auto found = glyphs.find(character);
    return found == glyphs.end() ? unknown : found->second;
}

float distance(sf::Vector2f first, sf::Vector2f second) {
    const float dx = first.x - second.x;
    const float dy = first.y - second.y;
    return std::sqrt(dx * dx + dy * dy);
}
} // namespace

Application::Application()
    : window_(sf::VideoMode({WindowWidth, WindowHeight}), "AStarLab",
              sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close),
      randomEngine_(static_cast<unsigned>(
          std::chrono::steady_clock::now().time_since_epoch().count())),
      buttons_{{
          {{{42.f, 70.f}, {130.f, 40.f}}, "Run", Start},
          {{{184.f, 70.f}, {145.f, 40.f}}, "Random", Path},
          {{{341.f, 70.f}, {130.f, 40.f}}, "Obstacles", ObstacleBorder},
          {{{483.f, 70.f}, {110.f, 40.f}}, "Learn A*", Expanded},
      }} {
    if (!font_.openFromFile(ASTARLAB_FONT_PATH)) {
        throw std::runtime_error("Could not load the bundled Inter font");
    }
    window_.setFramerateLimit(120);
    window_.setMinimumSize(sf::Vector2u{960, 645});
    updateResponsiveView(window_.getSize());
    buildRoadmap(false);
    initialCircles_ = {{{380.f, 270.f}, 58.f}, {{690.f, 155.f}, 45.f}};
    initialRectangles_ = {{{545.f, 370.f}, {125.f, 82.f}}};
    resetScene();
}

void Application::buildRoadmap(bool randomized) {
    graph_.clear();
    const std::array<sf::Vector2f, 23> basePositions{{
        {60, 90}, {190, 50}, {335, 85}, {480, 45}, {625, 85}, {790, 55},
        {115, 205}, {260, 195}, {410, 210}, {555, 190}, {710, 225}, {850, 175},
        {55, 350}, {205, 340}, {350, 365}, {500, 335}, {655, 355}, {825, 325},
        {135, 500}, {300, 485}, {465, 510}, {640, 490}, {850, 490},
    }};
    auto positions = basePositions;

    if (randomized) {
        std::uniform_real_distribution<float> jitter(-32.f, 32.f);
        for (std::size_t index = 0; index < positions.size(); ++index) {
            positions[index].x = std::clamp(basePositions[index].x + jitter(randomEngine_),
                                            35.f, CanvasWidth - 35.f);
            positions[index].y = std::clamp(basePositions[index].y + jitter(randomEngine_),
                                            35.f, CanvasHeight - 35.f);
        }
    }

    for (const auto position : positions) graph_.addNode(position.x, position.y);

    std::uniform_int_distribution<int> extraCost(0, randomized ? 8 : 0);
    // Keep the original roadmap topology so every random graph has the same
    // readable, well-connected structure. Only positions and weights vary.
    for (std::size_t first = 0; first < positions.size(); ++first) {
        for (std::size_t second = first + 1; second < positions.size(); ++second) {
            if (distance(basePositions[first], basePositions[second]) > 178.f) continue;
            const int geometricCost = static_cast<int>(
                std::ceil(distance(positions[first], positions[second]) / 10.f));
            graph_.addUndirectedEdge(static_cast<int>(first), static_cast<int>(second),
                                     geometricCost + extraCost(randomEngine_));
        }
    }
}

void Application::randomizeRoadmap() {
    buildRoadmap(true);
    obstacleTool_ = ObstacleTool::Move;
    obstacleMenuOpen_ = false;
    recomputeEdgeAvailability();
    clearSearch("New nodes and edge costs generated — run the search");
}

void Application::resetScene() {
    buildRoadmap(false);
    circles_ = initialCircles_;
    rectangles_ = initialRectangles_;
    obstacleTool_ = ObstacleTool::Move;
    obstacleMenuOpen_ = false;
    recomputeEdgeAvailability();
    clearSearch("Ready — move an obstacle or run the search");
}

void Application::recomputeEdgeAvailability() {
    for (auto& edge : graph_.edges()) edge.enabled = !edgeIntersectsObstacle(edge);
}

void Application::clearSearch(const std::string& status) {
    result_ = {};
    expandedNodes_.clear();
    visiblePathNodes_.clear();
    scores_.clear();
    currentScore_.reset();
    currentNeighbor_ = -1;
    activePseudocodeLine_ = -1;
    animationIndex_ = 0;
    animationAccumulator_ = sf::Time::Zero;
    animationPhase_ = AnimationPhase::Idle;
    status_ = status;
}

void Application::startSearch() {
    clearSearch("A* is choosing the node with the lowest f score");
    dragKind_ = DragKind::None;
    obstacleMenuOpen_ = false;
    result_ = astar_.search(graph_, startNode_, goalNode_);
    animationPhase_ = AnimationPhase::Expanding;
}

void Application::cancelSearch() {
    dragKind_ = DragKind::None;
    obstacleMenuOpen_ = false;
    clearSearch("Simulation canceled — obstacles are unlocked");
}

bool Application::isSimulationRunning() const {
    return animationPhase_ == AnimationPhase::Expanding ||
           animationPhase_ == AnimationPhase::ShowingPath;
}

void Application::toggleTheme() {
    darkMode_ = !darkMode_;
    applyTheme(darkMode_);
    buttons_[0].accent = Start;
    buttons_[1].accent = Path;
    buttons_[2].accent = ObstacleBorder;
    buttons_[3].accent = Expanded;
    status_ = darkMode_ ? "Dark mode enabled" : "Light mode enabled";
}

void Application::toggleSpeed() {
    slowMode_ = !slowMode_;
    status_ = slowMode_ ? "Slower playback enabled" : "Faster playback enabled";
}

void Application::togglePseudocode() {
    showPseudocode_ = !showPseudocode_;
    status_ = showPseudocode_ ? "Synchronized pseudocode visible"
                              : "Synchronized pseudocode hidden";
}

void Application::toggleLearningView() {
    learningView_ = !learningView_;
    obstacleMenuOpen_ = false;
    buttons_[3].label = learningView_ ? "Back to lab" : "Learn A*";
    status_ = learningView_ ? "A* from the ground up" : "Back in the interactive lab";
}

void Application::addObstacleAt(sf::Vector2f local) {
    local.x = std::clamp(local.x, 30.f, CanvasWidth - 30.f);
    local.y = std::clamp(local.y, 30.f, CanvasHeight - 30.f);
    if (obstacleTool_ == ObstacleTool::AddCircle) {
        circles_.push_back({local, 42.f});
        status_ = "Circle added — drag it or run the search";
    } else if (obstacleTool_ == ObstacleTool::AddRectangle) {
        rectangles_.push_back({local - sf::Vector2f{52.f, 36.f}, {104.f, 72.f}});
        status_ = "Rectangle added — drag it or run the search";
    } else {
        return;
    }
    recomputeEdgeAvailability();
    result_ = {};
    expandedNodes_.clear();
    visiblePathNodes_.clear();
    scores_.clear();
    currentScore_.reset();
    currentNeighbor_ = -1;
    activePseudocodeLine_ = -1;
    animationPhase_ = AnimationPhase::Idle;
    obstacleTool_ = ObstacleTool::Move;
}

bool Application::deleteObstacleAt(sf::Vector2f local) {
    for (std::size_t index = circles_.size(); index > 0; --index) {
        if (distance(local, circles_[index - 1].center) <= circles_[index - 1].radius) {
            circles_.erase(circles_.begin() + static_cast<std::ptrdiff_t>(index - 1));
            recomputeEdgeAvailability();
            clearSearch("Obstacle deleted — run the search again");
            obstacleTool_ = ObstacleTool::Move;
            return true;
        }
    }
    for (std::size_t index = rectangles_.size(); index > 0; --index) {
        const sf::FloatRect bounds(rectangles_[index - 1].position, rectangles_[index - 1].size);
        if (bounds.contains(local)) {
            rectangles_.erase(rectangles_.begin() + static_cast<std::ptrdiff_t>(index - 1));
            recomputeEdgeAvailability();
            clearSearch("Obstacle deleted — run the search again");
            obstacleTool_ = ObstacleTool::Move;
            return true;
        }
    }
    return false;
}

void Application::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        processEvents();
        update(clock.restart());
        render();
    }
}

void Application::processEvents() {
    while (const auto event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
        } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            updateResponsiveView(resized->size);
        } else if (const auto* pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (pressed->button == sf::Mouse::Button::Left ||
                pressed->button == sf::Mouse::Button::Right) {
                handleMousePressed(pressed->position, pressed->button);
            }
        } else if (const auto* moved = event->getIf<sf::Event::MouseMoved>()) {
            handleMouseMoved(moved->position);
        } else if (event->is<sf::Event::MouseButtonReleased>()) {
            dragKind_ = DragKind::None;
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            handleKeyPressed(key->scancode);
        }
    }
}

void Application::updateResponsiveView(sf::Vector2u windowSize) {
    if (windowSize.x == 0 || windowSize.y == 0) return;

    uiView_ = sf::View(sf::FloatRect({0.f, 0.f},
                                    {static_cast<float>(WindowWidth),
                                     static_cast<float>(WindowHeight)}));

    const float windowAspect = static_cast<float>(windowSize.x) /
                               static_cast<float>(windowSize.y);
    const float designAspect = static_cast<float>(WindowWidth) /
                               static_cast<float>(WindowHeight);
    sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});

    if (windowAspect > designAspect) {
        viewport.size.x = designAspect / windowAspect;
        viewport.position.x = (1.f - viewport.size.x) / 2.f;
    } else {
        viewport.size.y = windowAspect / designAspect;
        viewport.position.y = (1.f - viewport.size.y) / 2.f;
    }

    uiView_.setViewport(viewport);
    window_.setView(uiView_);
}

void Application::handleMousePressed(sf::Vector2i pixel, sf::Mouse::Button button) {
    const sf::Vector2f screen = window_.mapPixelToCoords(pixel, uiView_);
    const bool simulationRunning = isSimulationRunning();
    if (button == sf::Mouse::Button::Left) {
        for (std::size_t index = 0; index < buttons_.size(); ++index) {
            if (!contains(buttons_[index].bounds, screen)) continue;
            if (index == 0 && !learningView_) {
                if (simulationRunning) cancelSearch();
                else startSearch();
            }
            if (index == 1 && !learningView_ && !simulationRunning) randomizeRoadmap();
            if (index == 2 && !learningView_ && !simulationRunning) {
                obstacleMenuOpen_ = !obstacleMenuOpen_;
            }
            if (index == 3 && !simulationRunning) toggleLearningView();
            return;
        }

        const sf::FloatRect resetControl({770.f, 74.f}, {70.f, 32.f});
        const sf::FloatRect themeControl({856.f, 74.f}, {100.f, 32.f});
        const sf::FloatRect speedControl({972.f, 74.f}, {112.f, 32.f});
        const sf::FloatRect codeControl({1100.f, 74.f}, {130.f, 32.f});
        if (resetControl.contains(screen) && !learningView_ && !simulationRunning) {
            resetScene();
            return;
        }
        if (themeControl.contains(screen)) {
            toggleTheme();
            return;
        }
        if (speedControl.contains(screen)) {
            toggleSpeed();
            return;
        }
        if (codeControl.contains(screen) && !learningView_) {
            togglePseudocode();
            return;
        }

        if (obstacleMenuOpen_ && !learningView_) {
            const sf::FloatRect circleItem({341.f, 118.f}, {170.f, 34.f});
            const sf::FloatRect rectangleItem({341.f, 152.f}, {170.f, 34.f});
            const sf::FloatRect deleteItem({341.f, 186.f}, {170.f, 34.f});
            if (circleItem.contains(screen)) {
                obstacleTool_ = ObstacleTool::AddCircle;
                obstacleMenuOpen_ = false;
                status_ = "Circle tool selected — click inside the graph";
                return;
            }
            if (rectangleItem.contains(screen)) {
                obstacleTool_ = ObstacleTool::AddRectangle;
                obstacleMenuOpen_ = false;
                status_ = "Rectangle tool selected — click inside the graph";
                return;
            }
            if (deleteItem.contains(screen)) {
                obstacleTool_ = ObstacleTool::Delete;
                obstacleMenuOpen_ = false;
                status_ = "Delete tool selected — click an obstacle";
                return;
            }
            obstacleMenuOpen_ = false;
        }
    }

    const sf::Vector2f local = toLocal(pixel);
    if (learningView_) {
        const float learningWidth = static_cast<float>(WindowWidth) - CanvasLeft * 2.f;
        const sf::FloatRect previous({30.f, CanvasHeight - 62.f}, {120.f, 40.f});
        const sf::FloatRect next({learningWidth - 150.f, CanvasHeight - 62.f}, {120.f, 40.f});
        if (button == sf::Mouse::Button::Left && previous.contains(local)) {
            learningPage_ = std::max(0, learningPage_ - 1);
        }
        if (button == sf::Mouse::Button::Left && next.contains(local)) {
            learningPage_ = std::min(3, learningPage_ + 1);
        }
        return;
    }

    if (simulationRunning) {
        status_ = "Cancel the simulation before editing obstacles";
        return;
    }

    if (local.x < 0.f || local.y < 0.f || local.x > CanvasWidth || local.y > CanvasHeight) return;
    if (button == sf::Mouse::Button::Right || obstacleTool_ == ObstacleTool::Delete) {
        deleteObstacleAt(local);
        return;
    }
    if (obstacleTool_ == ObstacleTool::AddCircle ||
        obstacleTool_ == ObstacleTool::AddRectangle) {
        addObstacleAt(local);
        return;
    }

    for (std::size_t index = 0; index < circles_.size(); ++index) {
        if (distance(local, circles_[index].center) <= circles_[index].radius) {
            dragKind_ = DragKind::Circle;
            draggedIndex_ = index;
            dragOffset_ = local - circles_[index].center;
            return;
        }
    }
    for (std::size_t index = 0; index < rectangles_.size(); ++index) {
        const sf::FloatRect bounds(rectangles_[index].position, rectangles_[index].size);
        if (bounds.contains(local)) {
            dragKind_ = DragKind::Rectangle;
            draggedIndex_ = index;
            dragOffset_ = local - rectangles_[index].position;
            return;
        }
    }
}

void Application::handleMouseMoved(sf::Vector2i pixel) {
    if (isSimulationRunning()) {
        dragKind_ = DragKind::None;
        return;
    }
    if (dragKind_ == DragKind::None) return;
    sf::Vector2f local = toLocal(pixel);
    local.x = std::clamp(local.x, 0.f, CanvasWidth);
    local.y = std::clamp(local.y, 0.f, CanvasHeight);

    if (dragKind_ == DragKind::Circle) circles_[draggedIndex_].center = local - dragOffset_;
    if (dragKind_ == DragKind::Rectangle) rectangles_[draggedIndex_].position = local - dragOffset_;
    recomputeEdgeAvailability();
    clearSearch("The graph changed — run the search again");
}

void Application::handleKeyPressed(sf::Keyboard::Scancode key) {
    if (key == sf::Keyboard::Scancode::Escape) window_.close();
    if (!learningView_ &&
        (key == sf::Keyboard::Scancode::Space || key == sf::Keyboard::Scancode::Enter)) {
        if (isSimulationRunning()) cancelSearch();
        else startSearch();
    }
    if (key == sf::Keyboard::Scancode::R && !learningView_ && !isSimulationRunning()) resetScene();
    if (key == sf::Keyboard::Scancode::G && !learningView_ && !isSimulationRunning()) randomizeRoadmap();
    if (key == sf::Keyboard::Scancode::T) toggleTheme();
    if (key == sf::Keyboard::Scancode::S) toggleSpeed();
    if (key == sf::Keyboard::Scancode::P && !learningView_) togglePseudocode();
    if (key == sf::Keyboard::Scancode::L && !isSimulationRunning()) toggleLearningView();
    if (learningView_ && key == sf::Keyboard::Scancode::Left) {
        learningPage_ = std::max(0, learningPage_ - 1);
    }
    if (learningView_ && key == sf::Keyboard::Scancode::Right) {
        learningPage_ = std::min(3, learningPage_ + 1);
    }
}

void Application::update(sf::Time elapsed) {
    if (learningView_) return;
    if (animationPhase_ != AnimationPhase::Expanding &&
        animationPhase_ != AnimationPhase::ShowingPath) return;

    animationAccumulator_ += elapsed;
    const sf::Time step = animationPhase_ == AnimationPhase::Expanding
                              ? sf::milliseconds(slowMode_ ? 520 : 150)
                              : sf::milliseconds(slowMode_ ? 360 : 120);
    while (animationAccumulator_ >= step) {
        animationAccumulator_ -= step;
        if (animationPhase_ == AnimationPhase::Expanding) {
            if (animationIndex_ < result_.steps.size()) {
                applySearchStep(result_.steps[animationIndex_++]);
            } else if (result_.found) {
                animationPhase_ = AnimationPhase::ShowingPath;
                animationIndex_ = 0;
                status_ = "Goal reached — tracing the lowest-cost path";
            } else {
                animationPhase_ = AnimationPhase::Complete;
                status_ = "No path — the obstacles disconnected the graph";
            }
        } else if (animationIndex_ < result_.path.size()) {
            visiblePathNodes_.insert(result_.path[animationIndex_++]);
        } else {
            animationPhase_ = AnimationPhase::Complete;
            std::ostringstream message;
            message << "Best path: cost " << result_.cost << "   ·   explored "
                    << result_.expanded.size() << " nodes";
            status_ = message.str();
        }
    }
}

void Application::applySearchStep(const SearchStep& step) {
    activePseudocodeLine_ = step.pseudocodeLine;
    currentNeighbor_ = step.neighborNode;
    std::ostringstream message;

    switch (step.type) {
    case SearchStepType::Initialize:
        scores_[step.currentNode] = step.score;
        currentScore_ = step.score;
        message << "Start at node " << step.currentNode << " and add it to open";
        break;
    case SearchStepType::BeginLoop:
        currentNeighbor_ = -1;
        message << "The open set is not empty — continue searching";
        break;
    case SearchStepType::SelectCurrent:
        expandedNodes_.insert(step.currentNode);
        scores_[step.currentNode] = step.score;
        currentScore_ = step.score;
        message << "Choose node " << step.currentNode << " with f=" << step.score.f;
        break;
    case SearchStepType::CheckGoal:
        currentScore_ = step.score;
        message << "Is node " << step.currentNode << " the goal? "
                << (step.improved ? "Yes" : "No");
        break;
    case SearchStepType::InspectNeighbor:
        message << "Inspect neighbor " << step.neighborNode << " (edge cost "
                << step.edgeCost << ")";
        break;
    case SearchStepType::CalculateTentative:
        message << "New g for node " << step.neighborNode << " = " << step.score.g
                << " + " << step.edgeCost << " = " << step.tentativeG;
        break;
    case SearchStepType::CompareCost:
        message << (step.improved ? "This route is cheaper" : "Keep the existing route");
        if (step.previousG >= 0) message << " (old g=" << step.previousG << ")";
        break;
    case SearchStepType::UpdateNeighbor:
        scores_[step.neighborNode] = step.score;
        message << "Update node " << step.neighborNode << ": g=" << step.score.g
                << ", h=" << step.score.h << ", f=" << step.score.f;
        break;
    case SearchStepType::GoalReached:
        currentNeighbor_ = -1;
        message << "Goal reached — rebuild the path using parents";
        break;
    case SearchStepType::SearchExhausted:
        currentNeighbor_ = -1;
        message << "Open is empty, so no path exists";
        break;
    }
    status_ = message.str();
}

void Application::render() {
    window_.clear(Background);
    if (learningView_) {
        drawLearningView();
    } else {
        drawRoadmap();
        drawObstacles();
        if (showPseudocode_) drawSidebar();
        drawLegend();
    }
    drawHeader();
    window_.display();
}

void Application::drawHeader() {
    drawText("AStarLab", {42.f, 18.f}, 3.f, Text, true);
    drawText("Explore how A* finds the lowest-cost route", {200.f, 27.f}, 1.7f, Muted);
    const bool simulationRunning = isSimulationRunning();
    for (std::size_t index = 0; index < buttons_.size(); ++index) {
        const auto& button = buttons_[index];
        const bool active = (index == 0 && simulationRunning && !learningView_) ||
                            (index == 2 && obstacleMenuOpen_) ||
                            (index == 3 && learningView_);
        const bool unavailable = (learningView_ && index < 3) ||
                                 (simulationRunning && index > 0);
        const std::string label = index == 0 && simulationRunning
                                      ? "Cancel"
                                      : button.label;
        const sf::Color accent = index == 0 && simulationRunning ? Goal : button.accent;
        sf::RectangleShape shape(button.bounds.size);
        shape.setPosition(button.bounds.position);
        shape.setFillColor(active ? accent : Panel);
        shape.setOutlineThickness(1.f);
        shape.setOutlineColor(active ? accent : Edge);
        window_.draw(shape);
        drawText(label, button.bounds.position + sf::Vector2f{14.f, 10.f}, 2.f,
                 unavailable ? Muted : (active ? Panel : Text), true);
    }

    const std::array<std::tuple<sf::FloatRect, std::string, sf::Color>, 4> utilities{{
        {{{770.f, 74.f}, {70.f, 32.f}}, "Reset", simulationRunning ? Edge : Muted},
        {{{856.f, 74.f}, {100.f, 32.f}}, darkMode_ ? "Light" : "Dark", Muted},
        {{{972.f, 74.f}, {112.f, 32.f}}, slowMode_ ? "Faster" : "Slower", Expanded},
        {{{1100.f, 74.f}, {130.f, 32.f}}, showPseudocode_ ? "Hide code" : "Show code", Path},
    }};
    for (const auto& [bounds, label, accent] : utilities) {
        sf::RectangleShape control(bounds.size);
        control.setPosition(bounds.position);
        control.setFillColor(PanelLight);
        window_.draw(control);
        drawText(label, bounds.position + sf::Vector2f{11.f, 7.f}, 1.45f, accent, true);
    }

    drawText(status_, {610.f, 128.f}, 1.2f,
             status_.find("No path") == 0 ? Goal : Text);

    if (obstacleMenuOpen_ && !learningView_) {
        sf::RectangleShape menu({170.f, 102.f});
        menu.setPosition({341.f, 118.f});
        menu.setFillColor(PanelLight);
        menu.setOutlineThickness(1.f);
        menu.setOutlineColor(Edge);
        window_.draw(menu);
        drawText("Add circle", {355.f, 123.f}, 1.45f, Text);
        drawText("Add rectangle", {355.f, 157.f}, 1.45f, Text);
        drawText("Delete obstacle", {355.f, 191.f}, 1.45f, Goal);
    }
}

void Application::drawRoadmap() {
    sf::RectangleShape canvas({CanvasWidth, CanvasHeight});
    canvas.setPosition({CanvasLeft, CanvasTop});
    canvas.setFillColor(Panel);
    canvas.setOutlineThickness(1.f);
    canvas.setOutlineColor(Edge);
    window_.draw(canvas);

    for (const auto& edge : graph_.edges()) {
        const sf::Vector2f from = toScreen(graph_.node(edge.from));
        const sf::Vector2f to = toScreen(graph_.node(edge.to));
        const bool inspecting = currentScore_ && currentNeighbor_ >= 0 &&
            ((edge.from == currentScore_->nodeId && edge.to == currentNeighbor_) ||
             (edge.to == currentScore_->nodeId && edge.from == currentNeighbor_));
        drawLine(from, to, inspecting ? 5.f : (edge.enabled ? 2.f : 1.f),
                 inspecting ? Path : (edge.enabled ? Edge : BlockedEdge));

        const sf::Vector2f midpoint = (from + to) / 2.f;
        sf::RectangleShape costBackground({24.f, 15.f});
        costBackground.setPosition(midpoint - sf::Vector2f{12.f, 8.f});
        costBackground.setFillColor(Panel);
        window_.draw(costBackground);
        const std::string cost = std::to_string(edge.cost);
        drawText(cost, midpoint - sf::Vector2f{cost.size() * 3.3f, 5.f}, 1.1f,
                 edge.enabled ? Muted : BlockedEdge);
    }

    if (visiblePathNodes_.size() >= 2) {
        for (std::size_t index = 1; index < result_.path.size(); ++index) {
            if (visiblePathNodes_.count(result_.path[index - 1]) == 0U ||
                visiblePathNodes_.count(result_.path[index]) == 0U) continue;
            drawLine(toScreen(graph_.node(result_.path[index - 1])),
                     toScreen(graph_.node(result_.path[index])), 6.f, Path);
        }
    }

    for (const auto& node : graph_.nodes()) drawNode(node.id);
}

void Application::drawObstacles() {
    for (const auto& obstacle : circles_) {
        sf::CircleShape shape(obstacle.radius);
        shape.setOrigin({obstacle.radius, obstacle.radius});
        shape.setPosition({CanvasLeft + obstacle.center.x, CanvasTop + obstacle.center.y});
        shape.setFillColor(Obstacle);
        shape.setOutlineThickness(3.f);
        shape.setOutlineColor(ObstacleBorder);
        window_.draw(shape);
        drawText("Move me", {CanvasLeft + obstacle.center.x - 26.f,
                             CanvasTop + obstacle.center.y - 8.f}, 1.3f, Text, true);
    }
    for (const auto& obstacle : rectangles_) {
        sf::RectangleShape shape(obstacle.size);
        shape.setPosition({CanvasLeft + obstacle.position.x, CanvasTop + obstacle.position.y});
        shape.setFillColor(Obstacle);
        shape.setOutlineThickness(3.f);
        shape.setOutlineColor(ObstacleBorder);
        window_.draw(shape);
        drawText("Move me", {CanvasLeft + obstacle.position.x + obstacle.size.x / 2.f - 26.f,
                             CanvasTop + obstacle.position.y + obstacle.size.y / 2.f - 8.f},
                 1.3f, Text, true);
    }
}

void Application::drawSidebar() {
    const float left = 982.f;
    if (showPseudocode_) {
        drawText("A* pseudocode", {left, 180.f}, 2.1f, Text, true);
        drawText("The active line follows the search", {left, 211.f}, 1.05f, Muted);

        const std::array<std::string, 9> pseudocode{{
            "1  open = {start}",
            "2  while open not empty",
            "3    current = lowest f",
            "4    if current is goal",
            "5    for each neighbor",
            "6      newG = g + cost",
            "7      if newG < old g",
            "8        update g, h, f",
            "9  rebuild best path",
        }};
        float lineTop = 237.f;
        for (std::size_t index = 0; index < pseudocode.size(); ++index) {
            const bool active = activePseudocodeLine_ == static_cast<int>(index + 1);
            if (active) {
                sf::RectangleShape highlight({255.f, 28.f});
                highlight.setPosition({left, lineTop - 4.f});
                highlight.setFillColor(Expanded);
                window_.draw(highlight);
            }
            drawText(pseudocode[index], {left + 8.f, lineTop}, 1.05f,
                     active ? Panel : Text, active);
            lineTop += 31.f;
        }
    } else {
        drawText("Visualization focus", {left, 180.f}, 2.1f, Text, true);
        drawText("Pseudocode is hidden", {left, 216.f}, 1.25f, Muted);
        drawText("Use Show code to follow each line", {left, 242.f}, 1.05f, Muted);
    }

    const float liveTop = showPseudocode_ ? 532.f : 285.f;
    sf::RectangleShape current({255.f, 118.f});
    current.setPosition({left, liveTop});
    current.setFillColor(PanelLight);
    current.setOutlineThickness(1.f);
    current.setOutlineColor(currentScore_ ? Expanded : Edge);
    window_.draw(current);
    drawText("Live values", {left + 14.f, liveTop + 12.f}, 1.25f, Muted, true);
    if (currentScore_) {
        std::string nodes = "Current  " + std::to_string(currentScore_->nodeId);
        if (currentNeighbor_ >= 0) nodes += "     Neighbor  " + std::to_string(currentNeighbor_);
        drawText(nodes, {left + 14.f, liveTop + 38.f}, 1.25f, Text, true);
        drawText("g " + std::to_string(currentScore_->g), {left + 14.f, liveTop + 74.f}, 1.55f, Start, true);
        drawText("h " + std::to_string(currentScore_->h), {left + 91.f, liveTop + 74.f}, 1.55f, Goal, true);
        drawText("f " + std::to_string(currentScore_->f), {left + 168.f, liveTop + 74.f}, 1.55f, Path, true);
    } else {
        drawText("Run the search to begin", {left + 14.f, liveTop + 49.f}, 1.25f, Muted);
    }

    drawText("Space  Run     S  Speed", {left, 681.f}, 1.2f, Muted);
    drawText("R  Reset       G  Random", {left, 704.f}, 1.2f, Muted);
    drawText("T  Theme       P  Code       L  Learn", {left, 727.f}, 1.05f, Muted);
}

void Application::drawLegend() {
    const float top = 780.f;
    const std::array<std::pair<sf::Color, std::string>, 5> items{{
        {Start, "Start"}, {Goal, "Goal"}, {Expanded, "Explored"},
        {Path, "Best path"}, {ObstacleBorder, "Movable obstacle"},
    }};
    float left = CanvasLeft;
    for (const auto& [color, label] : items) {
        sf::CircleShape dot(6.f);
        dot.setPosition({left, top});
        dot.setFillColor(color);
        window_.draw(dot);
        drawText(label, {left + 19.f, top + 1.f}, 1.3f, Muted);
        left += label.size() * 8.f + 70.f;
    }
    drawText("Soft red edges are blocked by an obstacle", {CanvasLeft, 814.f}, 1.2f, BlockedEdge);
}

void Application::drawLearningView() {
    const float width = static_cast<float>(WindowWidth) - CanvasLeft * 2.f;
    sf::RectangleShape page({width, CanvasHeight});
    page.setPosition({CanvasLeft, CanvasTop});
    page.setFillColor(Panel);
    page.setOutlineThickness(1.f);
    page.setOutlineColor(Edge);
    window_.draw(page);

    drawText("Lesson " + std::to_string(learningPage_ + 1) + " of 4",
             {CanvasLeft + 32.f, CanvasTop + 24.f}, 1.35f, Muted, true);

    for (int index = 0; index < 4; ++index) {
        sf::CircleShape dot(index == learningPage_ ? 6.f : 4.f);
        dot.setPosition({CanvasLeft + width - 100.f + index * 22.f,
                         CanvasTop + (index == learningPage_ ? 25.f : 27.f)});
        dot.setFillColor(index == learningPage_ ? Expanded : Edge);
        window_.draw(dot);
    }

    const float left = CanvasLeft + 55.f;
    const float top = CanvasTop + 70.f;

    if (learningPage_ == 0) {
        drawText("1. Turn the world into a graph", {left, top}, 2.7f, Text, true);
        drawText("A* does not search empty space directly. It searches a graph.",
                 {left, top + 52.f}, 1.6f, Muted);

        const std::array<std::pair<std::string, std::string>, 3> concepts{{
            {"Nodes", "Possible positions the search may visit"},
            {"Edges", "Allowed moves between two nodes"},
            {"Cost", "The price of taking an edge"},
        }};
        float cardTop = top + 105.f;
        for (const auto& [name, explanation] : concepts) {
            sf::RectangleShape card({490.f, 62.f});
            card.setPosition({left, cardTop});
            card.setFillColor(PanelLight);
            window_.draw(card);
            drawText(name, {left + 18.f, cardTop + 13.f}, 1.8f, Text, true);
            drawText(explanation, {left + 125.f, cardTop + 17.f}, 1.35f, Muted);
            cardTop += 76.f;
        }

        const sf::Vector2f first{left + 680.f, top + 195.f};
        const sf::Vector2f second{left + 940.f, top + 195.f};
        drawLine(first, second, 5.f, Edge);
        sf::CircleShape firstNode(24.f);
        firstNode.setOrigin({24.f, 24.f});
        firstNode.setPosition(first);
        firstNode.setFillColor(Start);
        window_.draw(firstNode);
        sf::CircleShape secondNode(24.f);
        secondNode.setOrigin({24.f, 24.f});
        secondNode.setPosition(second);
        secondNode.setFillColor(Goal);
        window_.draw(secondNode);
        drawText("Start", {first.x - 24.f, first.y + 34.f}, 1.35f, Muted);
        drawText("Goal", {second.x - 20.f, second.y + 34.f}, 1.35f, Muted);
        drawText("edge cost = 26", {(first.x + second.x) / 2.f - 50.f, first.y - 42.f},
                 1.5f, Path, true);
        drawText("A* minimizes total edge cost — not simply the number of nodes.",
                 {left, top + 380.f}, 1.5f, Text, true);
    } else if (learningPage_ == 1) {
        drawText("2. Understand the three scores", {left, top}, 2.7f, Text, true);
        drawText("Every candidate node receives a priority:  f(n) = g(n) + h(n)",
                 {left, top + 52.f}, 1.6f, Muted);

        const std::array<std::tuple<std::string, std::string, sf::Color>, 3> scores{{
            {"g(n)", "Exact cost from the start to n", Start},
            {"h(n)", "Estimated cost from n to the goal", Goal},
            {"f(n)", "Estimated total cost through n", Path},
        }};
        float cardLeft = left;
        for (const auto& [name, explanation, color] : scores) {
            sf::RectangleShape card({330.f, 145.f});
            card.setPosition({cardLeft, top + 115.f});
            card.setFillColor(PanelLight);
            card.setOutlineThickness(2.f);
            card.setOutlineColor(color);
            window_.draw(card);
            drawText(name, {cardLeft + 22.f, top + 137.f}, 2.6f, color, true);
            drawText(explanation, {cardLeft + 22.f, top + 195.f}, 1.25f, Text);
            cardLeft += 355.f;
        }

        drawText("A* expands the frontier node with the smallest f value.",
                 {left, top + 305.f}, 1.7f, Text, true);
        drawText("In AStarLab, h is straight-line distance scaled to the edge-cost units.",
                 {left, top + 350.f}, 1.45f, Muted);
        drawText("Because h never overestimates the true remaining cost, it is admissible.",
                 {left, top + 380.f}, 1.45f, Muted);
        drawText("That is what allows A* to guarantee an optimal path.",
                 {left, top + 410.f}, 1.45f, Muted);
    } else if (learningPage_ == 2) {
        drawText("3. Follow the A* search loop", {left, top}, 2.7f, Text, true);
        const std::array<std::string, 5> steps{{
            "1   Put the start node in the open set.",
            "2   Remove the open node with the lowest f score.",
            "3   If it is the goal, reconstruct the path and stop.",
            "4   Try each neighbor and update it when a cheaper g is found.",
            "5   Move the current node to the closed set, then repeat.",
        }};
        float lineTop = top + 68.f;
        for (const auto& step : steps) {
            drawText(step, {left, lineTop}, 1.55f, Text);
            lineTop += 48.f;
        }

        sf::RectangleShape code({1040.f, 135.f});
        code.setPosition({left, top + 325.f});
        code.setFillColor(PanelLight);
        window_.draw(code);
        drawText("while open set is not empty", {left + 22.f, top + 343.f}, 1.35f, Muted, true);
        drawText("current = node with minimum f", {left + 50.f, top + 372.f}, 1.35f, Text);
        drawText("relax every edge from current", {left + 50.f, top + 401.f}, 1.35f, Text);
        drawText("remember each improved node's parent", {left + 50.f, top + 430.f}, 1.35f, Text);
    } else {
        drawText("4. Learn by experimenting", {left, top}, 2.7f, Text, true);
        drawText("Return to the lab and test one change at a time.",
                 {left, top + 52.f}, 1.6f, Muted);

        const std::array<std::string, 5> experiments{{
            "Move an obstacle onto the current best route, then rerun A*.",
            "Add a circle and observe which intersecting edges become blocked.",
            "Generate a random graph and compare edge costs before searching.",
            "Enable slow mode and predict which node will be explored next.",
            "Compare g, h, and f in the Current step panel.",
        }};
        float experimentTop = top + 105.f;
        for (std::size_t index = 0; index < experiments.size(); ++index) {
            sf::CircleShape number(15.f);
            number.setOrigin({15.f, 15.f});
            number.setPosition({left + 15.f, experimentTop + 12.f});
            number.setFillColor(Expanded);
            window_.draw(number);
            drawText(std::to_string(index + 1), {left + 10.f, experimentTop}, 1.2f, Panel, true);
            drawText(experiments[index], {left + 52.f, experimentTop}, 1.45f, Text);
            experimentTop += 58.f;
        }
        drawText("Key idea: A* combines known cost with an informed estimate.",
                 {left, top + 420.f}, 1.65f, Path, true);
    }

    const float navigationTop = CanvasTop + CanvasHeight - 62.f;
    sf::RectangleShape previous({120.f, 40.f});
    previous.setPosition({CanvasLeft + 30.f, navigationTop});
    previous.setFillColor(learningPage_ == 0 ? PanelLight : Panel);
    previous.setOutlineThickness(1.f);
    previous.setOutlineColor(Edge);
    window_.draw(previous);
    drawText("Previous", {CanvasLeft + 50.f, navigationTop + 9.f}, 1.6f,
             learningPage_ == 0 ? Muted : Text, true);

    sf::RectangleShape next({120.f, 40.f});
    next.setPosition({CanvasLeft + width - 150.f, navigationTop});
    next.setFillColor(learningPage_ == 3 ? PanelLight : Expanded);
    window_.draw(next);
    drawText(learningPage_ == 3 ? "Finished" : "Next",
             {CanvasLeft + width - 126.f, navigationTop + 9.f}, 1.6f,
             learningPage_ == 3 ? Muted : Panel, true);
    drawText("Use Left / Right arrows to change lessons", {CanvasLeft + width / 2.f - 150.f,
             navigationTop + 10.f}, 1.2f, Muted);
}

void Application::drawNode(int nodeId) {
    const GraphNode& node = graph_.node(nodeId);
    const sf::Vector2f position = toScreen(node);
    sf::Color color = Node;
    if (expandedNodes_.count(nodeId) != 0U) color = Expanded;
    if (visiblePathNodes_.count(nodeId) != 0U) color = Path;
    if (nodeId == startNode_) color = Start;
    if (nodeId == goalNode_) color = Goal;

    sf::CircleShape circle(NodeRadius);
    circle.setOrigin({NodeRadius, NodeRadius});
    circle.setPosition(position);
    circle.setFillColor(color);
    const bool isCurrent = currentScore_ && currentScore_->nodeId == nodeId;
    const bool isNeighbor = currentNeighbor_ == nodeId;
    circle.setOutlineThickness(isCurrent || isNeighbor ? 4.f : 2.f);
    circle.setOutlineColor(isCurrent ? Text : (isNeighbor ? Path : Edge));
    window_.draw(circle);

    const std::string label = std::to_string(nodeId);
    drawText(label, {position.x - label.size() * 3.6f, position.y - 8.f}, 1.2f, Text, true);
    const auto score = scores_.find(nodeId);
    if (score != scores_.end() && nodeId != startNode_ && nodeId != goalNode_) {
        const std::string values = "g" + std::to_string(score->second.g) +
                                   "  f" + std::to_string(score->second.f);
        drawText(values, {position.x - values.size() * 2.7f, position.y + 22.f}, 0.9f, Text);
    }
}

void Application::drawLine(sf::Vector2f from, sf::Vector2f to, float thickness, sf::Color color) {
    const sf::Vector2f delta = to - from;
    const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    sf::RectangleShape line({length, thickness});
    line.setPosition(from);
    line.setOrigin({0.f, thickness / 2.f});
    line.setRotation(sf::degrees(std::atan2(delta.y, delta.x) * 180.f / 3.14159265f));
    line.setFillColor(color);
    window_.draw(line);
}

void Application::drawText(const std::string& text, sf::Vector2f position, float pixelSize,
                           sf::Color color, bool bold) {
    const auto characterSize = static_cast<unsigned>(std::max(10.f, std::round(pixelSize * 9.f)));
    sf::Text label(font_, text, characterSize);
    label.setPosition(position);
    label.setFillColor(color);
    if (bold) label.setStyle(sf::Text::Bold);
    window_.draw(label);
}

sf::Vector2f Application::toScreen(const GraphNode& node) const {
    return {CanvasLeft + node.x, CanvasTop + node.y};
}

sf::Vector2f Application::toLocal(sf::Vector2i pixel) const {
    return window_.mapPixelToCoords(pixel, uiView_) - sf::Vector2f{CanvasLeft, CanvasTop};
}

bool Application::edgeIntersectsObstacle(const GraphEdge& edge) const {
    const GraphNode& first = graph_.node(edge.from);
    const GraphNode& second = graph_.node(edge.to);
    const sf::Vector2f start{first.x, first.y};
    const sf::Vector2f end{second.x, second.y};
    for (const auto& circle : circles_) {
        if (segmentIntersectsCircle(start, end, circle)) return true;
    }
    for (const auto& rectangle : rectangles_) {
        if (segmentIntersectsRectangle(start, end, rectangle)) return true;
    }
    return false;
}

bool Application::segmentIntersectsCircle(sf::Vector2f start, sf::Vector2f end,
                                          const CircleObstacle& circle) {
    const sf::Vector2f segment = end - start;
    const float lengthSquared = segment.x * segment.x + segment.y * segment.y;
    if (lengthSquared == 0.f) return distance(start, circle.center) <= circle.radius;
    const sf::Vector2f toCenter = circle.center - start;
    const float projection = std::clamp(
        (toCenter.x * segment.x + toCenter.y * segment.y) / lengthSquared, 0.f, 1.f);
    return distance(start + segment * projection, circle.center) <= circle.radius + 3.f;
}

bool Application::segmentIntersectsRectangle(sf::Vector2f start, sf::Vector2f end,
                                             const RectangleObstacle& rectangle) {
    const float left = rectangle.position.x - 3.f;
    const float right = rectangle.position.x + rectangle.size.x + 3.f;
    const float top = rectangle.position.y - 3.f;
    const float bottom = rectangle.position.y + rectangle.size.y + 3.f;
    const sf::Vector2f delta = end - start;
    float minimum = 0.f;
    float maximum = 1.f;

    const auto clip = [&minimum, &maximum](float p, float q) {
        if (p == 0.f) return q >= 0.f;
        const float ratio = q / p;
        if (p < 0.f) {
            if (ratio > maximum) return false;
            minimum = std::max(minimum, ratio);
        } else {
            if (ratio < minimum) return false;
            maximum = std::min(maximum, ratio);
        }
        return true;
    };

    return clip(-delta.x, start.x - left) && clip(delta.x, right - start.x) &&
           clip(-delta.y, start.y - top) && clip(delta.y, bottom - start.y);
}

bool Application::contains(const sf::FloatRect& bounds, sf::Vector2f point) {
    return bounds.contains(point);
}

