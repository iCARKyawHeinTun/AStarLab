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
#include <utility>
#include <vector>

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
sf::Color SyntaxKeyword{199, 160, 236};
sf::Color SyntaxFunction{120, 200, 232};
sf::Color SyntaxVariable{231, 201, 128};
sf::Color SyntaxOperator{180, 187, 197};
sf::Color OpenRing{240, 195, 90};

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
        SyntaxKeyword = {199, 160, 236};
        SyntaxFunction = {120, 200, 232};
        SyntaxVariable = {231, 201, 128};
        SyntaxOperator = {180, 187, 197};
        OpenRing = {240, 195, 90};
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
    SyntaxKeyword = {123, 69, 166};
    SyntaxFunction = {23, 108, 145};
    SyntaxVariable = {136, 92, 8};
    SyntaxOperator = {89, 101, 117};
    OpenRing = {183, 121, 31};
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

namespace {
constexpr std::array<SearchStrategy, 4> AllStrategies{
    SearchStrategy::AStar, SearchStrategy::Dijkstra, SearchStrategy::Greedy,
    SearchStrategy::WeightedAStar,
};
constexpr std::array<int, 5> LineDelayMs{1100, 650, 380, 180, 60};
constexpr std::array<int, 5> NodeDelayMs{1600, 1000, 600, 300, 120};

struct ChallengeText {
    const char* title;
    const char* task;
};
constexpr std::array<ChallengeText, 5> Challenges{{
    {"Detour", "Make the best path cost at least 20 more than with no obstacles at all."},
    {"One obstacle is enough", "With exactly ONE obstacle on the board, make the goal unreachable."},
    {"Greedy's mistake", "Make Greedy pay at least 15 more than A* for its path."},
    {"The heuristic pays off", "Make Dijkstra close at least twice as many nodes as A*."},
    {"Make A* sweat", "Keep a path, but make A* close at least 21 nodes."},
}};

const char* popName(SearchStrategy strategy) {
    switch (strategy) {
    case SearchStrategy::Dijkstra: return "popLowestG";
    case SearchStrategy::Greedy: return "popLowestH";
    default: return "popLowestF";
    }
}

std::string infinityOr(int value) { return value < 0 ? "infinity" : std::to_string(value); }

bool modifierDown(sf::Keyboard::Key left, sf::Keyboard::Key right) {
    return sf::Keyboard::isKeyPressed(left) || sf::Keyboard::isKeyPressed(right);
}
} // namespace

Application::Application()
    : window_(sf::VideoMode({WindowWidth, WindowHeight}), "AStarLab",
              sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close),
      randomEngine_(static_cast<unsigned>(
          std::chrono::steady_clock::now().time_since_epoch().count())) {
    if (!font_.openFromFile(ASTARLAB_FONT_PATH)) {
        throw std::runtime_error("Could not load the bundled Inter font");
    }
    // Row 1: playback and the main learning tools. Row 2: small utilities.
    buttons_ = {
        {ButtonId::Play, {{42.f, 70.f}, {104.f, 40.f}}, false},
        {ButtonId::Back, {{154.f, 70.f}, {72.f, 40.f}}, false},
        {ButtonId::Step, {{234.f, 70.f}, {88.f, 40.f}}, false},
        {ButtonId::Finish, {{330.f, 70.f}, {84.f, 40.f}}, false},
        {ButtonId::Strategy, {{432.f, 70.f}, {196.f, 40.f}}, false},
        {ButtonId::Predict, {{636.f, 70.f}, {150.f, 40.f}}, false},
        {ButtonId::Obstacles, {{794.f, 70.f}, {124.f, 40.f}}, false},
        {ButtonId::Learn, {{926.f, 70.f}, {110.f, 40.f}}, false},
        {ButtonId::Challenges, {{1044.f, 70.f}, {196.f, 40.f}}, false},
        {ButtonId::Reset, {{760.f, 124.f}, {66.f, 30.f}}, true},
        {ButtonId::Theme, {{834.f, 124.f}, {64.f, 30.f}}, true},
        {ButtonId::Speed, {{906.f, 124.f}, {88.f, 30.f}}, true},
        {ButtonId::StepBy, {{1002.f, 124.f}, {120.f, 30.f}}, true},
        {ButtonId::Code, {{1130.f, 124.f}, {110.f, 30.f}}, true},
    };
    window_.setFramerateLimit(120);
    window_.setMinimumSize(sf::Vector2u{960, 645});
    updateResponsiveView(window_.getSize());
    initialCircles_ = {{{380.f, 270.f}, 58.f}, {{690.f, 155.f}, 45.f}};
    initialRectangles_ = {{{545.f, 370.f}, {125.f, 82.f}}};
    resetScene();
}

// ===================================================================== scene

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
    invalidate("New node positions and edge costs. Press Play or Step.");
}

void Application::resetScene() {
    buildRoadmap(false);
    circles_ = initialCircles_;
    rectangles_ = initialRectangles_;
    startNode_ = 0;
    goalNode_ = 22;
    obstacleTool_ = ObstacleTool::Move;
    obstacleMenuOpen_ = false;
    recomputeEdgeAvailability();
    invalidate("Ready. Press Play, or Step through the search one line at a time.");
}

void Application::recomputeEdgeAvailability() {
    for (auto& edge : graph_.edges()) edge.enabled = !edgeIntersectsObstacle(edge);
}

void Application::addObstacleAt(sf::Vector2f local) {
    local.x = std::clamp(local.x, 30.f, CanvasWidth - 30.f);
    local.y = std::clamp(local.y, 30.f, CanvasHeight - 30.f);
    const bool circle = obstacleTool_ == ObstacleTool::AddCircle;
    if (circle) circles_.push_back({local, 42.f});
    else rectangles_.push_back({local - sf::Vector2f{52.f, 36.f}, {104.f, 72.f}});
    obstacleTool_ = ObstacleTool::Move;
    recomputeEdgeAvailability();
    invalidate(circle ? "Circle added. Drag it around, then run the search."
                      : "Rectangle added. Drag it around, then run the search.");
}

bool Application::deleteObstacleAt(sf::Vector2f local) {
    for (std::size_t index = circles_.size(); index > 0; --index) {
        if (distance(local, circles_[index - 1].center) <= circles_[index - 1].radius) {
            circles_.erase(circles_.begin() + static_cast<std::ptrdiff_t>(index - 1));
            obstacleTool_ = ObstacleTool::Move;
            recomputeEdgeAvailability();
            invalidate("Obstacle deleted. The search starts over.");
            return true;
        }
    }
    for (std::size_t index = rectangles_.size(); index > 0; --index) {
        const sf::FloatRect bounds(rectangles_[index - 1].position, rectangles_[index - 1].size);
        if (bounds.contains(local)) {
            rectangles_.erase(rectangles_.begin() + static_cast<std::ptrdiff_t>(index - 1));
            obstacleTool_ = ObstacleTool::Move;
            recomputeEdgeAvailability();
            invalidate("Obstacle deleted. The search starts over.");
            return true;
        }
    }
    return false;
}

// ===================================================================== playback

void Application::invalidate(const std::string& status) {
    pause();
    result_.reset();
    position_ = -1;
    awaiting_ = false;
    answered_ = false;
    tries_ = 0;
    comparison_.clear();
    derive();
    status_ = status;
    evaluateChallenges();
}

void Application::ensureResult() {
    if (!result_) result_ = astar_.search(graph_, startNode_, goalNode_, strategy_);
}

/// Rebuild everything shown on screen by replaying steps 0..position_.
/// A run has a few hundred steps, so this is instant, and it makes "step back" trivial.
void Application::derive() {
    openSet_.clear();
    closedNodes_.clear();
    closedScores_.clear();
    currentScore_.reset();
    currentNeighbor_ = -1;
    activePseudocodeLine_ = -1;
    lastStep_.reset();
    if (!result_) return;
    int sequence = 0;
    for (int index = 0; index <= position_; ++index) {
        const SearchStep& step = result_->steps[static_cast<std::size_t>(index)];
        activePseudocodeLine_ = step.pseudocodeLine;
        currentNeighbor_ = step.neighborNode;
        lastStep_ = step;
        switch (step.type) {
        case SearchStepType::Initialize:
            openSet_[step.currentNode] = {step.score, sequence++};
            currentScore_ = step.score;
            break;
        case SearchStepType::SelectCurrent:
            openSet_.erase(step.currentNode);
            closedNodes_.insert(step.currentNode);
            closedScores_[step.currentNode] = step.score;
            currentScore_ = step.score;
            break;
        case SearchStepType::CheckGoal:
            currentScore_ = step.score;
            break;
        case SearchStepType::UpdateNeighbor:
            openSet_[step.neighborNode] = {step.score, sequence++};
            break;
        case SearchStepType::BeginLoop:
        case SearchStepType::GoalReached:
        case SearchStepType::SearchExhausted:
            currentNeighbor_ = -1;
            break;
        default:
            break;
        }
    }
}

bool Application::atEnd() const {
    return result_ && position_ >= static_cast<int>(result_->steps.size()) - 1;
}

/// One press of "Step". Returns false when it stopped to ask for a prediction or reached the end.
bool Application::stepForward() {
    ensureResult();
    if (atEnd()) return false;
    for (;;) {
        if (atEnd()) break;
        const SearchStep& next = result_->steps[static_cast<std::size_t>(position_ + 1)];
        if (next.type == SearchStepType::SelectCurrent && predict_ && !answered_ && !openSet_.empty()) {
            ask();
            return false;
        }
        ++position_;
        if (next.type == SearchStepType::SelectCurrent) answered_ = false;
        derive();
        if (!stepByNode_ || next.type == SearchStepType::SelectCurrent ||
            next.type == SearchStepType::GoalReached || next.type == SearchStepType::SearchExhausted) {
            break;
        }
    }
    present();
    return !atEnd();
}

void Application::stepBack() {
    if (!result_ || position_ < 0) return;
    pause();
    awaiting_ = false;
    answered_ = false;
    if (!stepByNode_) {
        --position_;
    } else {
        int index = position_ - 1;
        while (index >= 0 && result_->steps[static_cast<std::size_t>(index)].type !=
                                 SearchStepType::SelectCurrent) {
            --index;
        }
        position_ = index;
    }
    derive();
    present();
}

void Application::restartPlayback() {
    ensureResult();
    pause();
    position_ = -1;
    awaiting_ = false;
    answered_ = false;
    derive();
    status_ = "Back at the start. Press Step or Play.";
}

void Application::finishPlayback() {
    ensureResult();
    pause();
    awaiting_ = false;
    position_ = static_cast<int>(result_->steps.size()) - 1;
    derive();
    present();
}

void Application::play() {
    ensureResult();
    if (atEnd()) {
        position_ = -1;
        derive();
    }
    if (awaiting_) return;
    playing_ = true;
    playAccumulator_ = sf::milliseconds(100000); // take the first step right away
}

void Application::pause() {
    playing_ = false;
    playAccumulator_ = sf::Time::Zero;
}

void Application::update(sf::Time elapsed) {
    if (learningView_ || !playing_) return;
    playAccumulator_ += elapsed;
    const auto& delays = stepByNode_ ? NodeDelayMs : LineDelayMs;
    const sf::Time delay = sf::milliseconds(delays[static_cast<std::size_t>(speed_ - 1)]);
    if (playAccumulator_ < delay) return;
    playAccumulator_ = sf::Time::Zero;
    const bool more = stepForward();
    if (awaiting_) {
        resumeAfterAnswer_ = true;
        pause();
        return;
    }
    if (!more) pause();
}

void Application::present() {
    if (atEnd()) {
        std::ostringstream message;
        if (result_->found) {
            message << "Done. " << strategyName(strategy_) << " found a path of cost " << result_->cost
                    << " and closed " << closedNodes_.size() << " nodes.";
            if (predict_ && predictTotal_ > 0) {
                message << "   Predictions: " << predictRight_ << "/" << predictTotal_ << " first try.";
            }
        } else {
            message << "Done. The obstacles disconnected the graph, so there is no path.";
        }
        status_ = message.str();
        computeComparison();
    } else if (lastStep_) {
        status_ = describeStep(*lastStep_);
    }
}

std::string Application::describeStep(const SearchStep& step) const {
    std::ostringstream message;
    switch (step.type) {
    case SearchStepType::Initialize:
        message << "Start: put node " << step.currentNode << " in the open set with g = 0.";
        break;
    case SearchStepType::BeginLoop:
        message << "The open set isn't empty, so keep searching.";
        break;
    case SearchStepType::SelectCurrent:
        message << "Take out node " << step.currentNode << ": it has the lowest " << priorityName()
                << " in the open set (" << step.score.f << "). Close it.";
        break;
    case SearchStepType::CheckGoal:
        message << "Is node " << step.currentNode << " the goal? " << (step.improved ? "Yes!" : "No.");
        break;
    case SearchStepType::InspectNeighbor:
        message << "Look at neighbor " << step.neighborNode << " (edge cost " << step.edgeCost << ").";
        break;
    case SearchStepType::CalculateTentative:
        message << "Through node " << step.currentNode << ", reaching " << step.neighborNode
                << " costs g = " << step.score.g << " + " << step.edgeCost << " = " << step.tentativeG << ".";
        break;
    case SearchStepType::CompareCost:
        if (step.improved) {
            message << step.tentativeG << " < " << infinityOr(step.previousG)
                    << (step.previousG < 0 ? " (never reached before)" : "") << ": this route is better.";
        } else {
            message << step.tentativeG << " is not cheaper than the known " << step.previousG
                    << ". Keep the old route.";
        }
        break;
    case SearchStepType::UpdateNeighbor:
        message << "Update node " << step.neighborNode << ": g = " << step.score.g << ", h = "
                << step.score.h << ", " << priorityName() << " = " << step.score.f
                << ". Parent = " << step.currentNode << ".";
        break;
    case SearchStepType::GoalReached:
        message << "Goal reached! Follow the parent links back to the start to get the path.";
        break;
    case SearchStepType::SearchExhausted:
        message << "The open set is empty: there is no path to the goal.";
        break;
    }
    return message.str();
}

std::vector<std::pair<int, Application::OpenEntry>> Application::sortedOpenSet() const {
    std::vector<std::pair<int, OpenEntry>> entries(openSet_.begin(), openSet_.end());
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.second.score.f != b.second.score.f) return a.second.score.f < b.second.score.f;
        return a.second.sequence < b.second.sequence;
    });
    return entries;
}

void Application::computeComparison() {
    comparison_.clear();
    for (const auto strategy : AllStrategies) {
        const auto result = astar_.search(graph_, startNode_, goalNode_, strategy);
        comparison_.push_back({strategy, result.found, result.cost, static_cast<int>(result.expanded.size())});
    }
}

// ===================================================================== predict mode

void Application::ask() {
    awaiting_ = true;
    status_ = std::string("Your turn: which open node (amber ring) will ") + popName(strategy_) +
              "() take out next? Click it.";
}

void Application::answer(int nodeId) {
    const auto open = sortedOpenSet();
    if (open.empty()) return;
    const int best = open.front().second.score.f;
    const auto chosen = openSet_.find(nodeId);
    if (chosen == openSet_.end()) {
        status_ = "Node " + std::to_string(nodeId) + " isn't in the open set. Only amber-ringed nodes can come out next.";
        return;
    }
    const int expected = result_->steps[static_cast<std::size_t>(position_ + 1)].currentNode;
    std::ostringstream message;
    if (chosen->second.score.f == best) {
        ++predictTotal_;
        if (tries_ == 0) ++predictRight_;
        message << "Right: node " << nodeId << " has the lowest " << priorityName() << " (" << best << ").";
        if (nodeId != expected) {
            message << " It's a tie; node " << expected << " goes first because it entered the open set earlier.";
        }
        finishAnswer(message.str());
        return;
    }
    ++tries_;
    if (tries_ >= 2) {
        ++predictTotal_;
        message << "Not node " << nodeId << " (" << priorityName() << " = " << chosen->second.score.f
                << "). The answer is node " << expected << ", with the lowest " << priorityName() << " = " << best << ".";
        finishAnswer(message.str());
        return;
    }
    message << "Not node " << nodeId << ": its " << priorityName() << " = " << chosen->second.score.f
            << ". Compare the values of all amber nodes. Which is smallest? Try again.";
    status_ = message.str();
}

void Application::finishAnswer(const std::string& message) {
    awaiting_ = false;
    answered_ = true;
    tries_ = 0;
    stepForward();
    status_ = message + "   Predictions: " + std::to_string(predictRight_) + "/" +
              std::to_string(predictTotal_) + " first try.";
    if (resumeAfterAnswer_) {
        resumeAfterAnswer_ = false;
        if (!atEnd()) {
            playing_ = true;
            playAccumulator_ = sf::Time::Zero;
        }
    }
}

// ===================================================================== challenges

void Application::evaluateChallenges() {
    const auto run = [this](SearchStrategy strategy) { return astar_.search(graph_, startNode_, goalNode_, strategy); };
    const auto astarResult = run(SearchStrategy::AStar);
    const auto greedy = run(SearchStrategy::Greedy);
    const auto dijkstra = run(SearchStrategy::Dijkstra);
    Graph open = graph_;
    for (auto& edge : open.edges()) edge.enabled = true;
    const auto free = astar_.search(open, startNode_, goalNode_);
    const int closedA = static_cast<int>(astarResult.expanded.size());
    const int closedD = static_cast<int>(dijkstra.expanded.size());

    challengeStatus_[0] = {astarResult.found && astarResult.cost >= free.cost + 20,
                           astarResult.found ? "cost " + std::to_string(astarResult.cost) + ", needs " +
                                                   std::to_string(free.cost + 20) + "+"
                                             : "no path at all"};
    challengeStatus_[1] = {obstacleCount() == 1 && !astarResult.found,
                           std::to_string(obstacleCount()) + " obstacle(s), goal " +
                               (astarResult.found ? "reachable" : "unreachable")};
    challengeStatus_[2] = {astarResult.found && greedy.found && greedy.cost >= astarResult.cost + 15,
                           astarResult.found ? "Greedy " + std::to_string(greedy.cost) + " vs A* " +
                                                   std::to_string(astarResult.cost)
                                             : "no path"};
    challengeStatus_[3] = {astarResult.found && closedD >= 2 * closedA,
                           astarResult.found ? "Dijkstra " + std::to_string(closedD) + " vs A* " +
                                                   std::to_string(closedA)
                                             : "no path"};
    challengeStatus_[4] = {astarResult.found && closedA >= 21,
                           astarResult.found ? "A* closed " + std::to_string(closedA) : "no path"};
    for (int index = 0; index < ChallengeCount; ++index) {
        if (challengeStatus_[static_cast<std::size_t>(index)].solved) challengesDone_[static_cast<std::size_t>(index)] = true;
    }
}

// ===================================================================== toggles

void Application::toggleTheme() {
    darkMode_ = !darkMode_;
    applyTheme(darkMode_);
    status_ = darkMode_ ? "Dark mode" : "Light mode";
}

void Application::cycleSpeed() {
    speed_ = speed_ % 5 + 1;
    status_ = "Playback speed " + std::to_string(speed_) + " of 5";
}

void Application::toggleStepBy() {
    stepByNode_ = !stepByNode_;
    status_ = stepByNode_ ? "Step now jumps one expanded node at a time"
                          : "Step now follows one pseudocode line at a time";
}

void Application::togglePseudocode() {
    showPseudocode_ = !showPseudocode_;
    status_ = showPseudocode_ ? "Pseudocode visible" : "Pseudocode hidden";
}

void Application::toggleLearningView() {
    pause();
    learningView_ = !learningView_;
    obstacleMenuOpen_ = false;
    status_ = learningView_ ? "A* from the ground up" : "Back in the interactive lab";
}

void Application::togglePredict() {
    predict_ = !predict_;
    predictRight_ = 0;
    predictTotal_ = 0;
    if (!predict_ && awaiting_) {
        awaiting_ = false;
        answered_ = true;
    }
    status_ = predict_ ? "Predict mode on: before each node is expanded, click the one you think comes out next."
                       : "Predict mode off.";
}

void Application::toggleChallenges() {
    showChallenges_ = !showChallenges_;
    evaluateChallenges();
    status_ = showChallenges_ ? "Change the graph until a challenge turns green. Then explain to yourself why."
                              : "Challenges hidden";
}

void Application::setStrategy(SearchStrategy strategy) {
    strategy_ = strategy;
    invalidate(std::string("Strategy: ") + strategyName(strategy_) + "   (open set sorted by " +
               priorityName() + "). Press Play or Step.");
}

void Application::cycleStrategy() {
    const auto found = std::find(AllStrategies.begin(), AllStrategies.end(), strategy_);
    const auto index = static_cast<std::size_t>(found - AllStrategies.begin());
    setStrategy(AllStrategies[(index + 1) % AllStrategies.size()]);
}

void Application::pressButton(ButtonId id) {
    if (!buttonEnabled(id)) return;
    switch (id) {
    case ButtonId::Play: playing_ ? pause() : play(); break;
    case ButtonId::Back: stepBack(); break;
    case ButtonId::Step: pause(); stepForward(); break;
    case ButtonId::Finish: finishPlayback(); break;
    case ButtonId::Strategy: cycleStrategy(); break;
    case ButtonId::Predict: togglePredict(); break;
    case ButtonId::Obstacles: obstacleMenuOpen_ = !obstacleMenuOpen_; break;
    case ButtonId::Learn: toggleLearningView(); break;
    case ButtonId::Challenges: toggleChallenges(); break;
    case ButtonId::Reset: resetScene(); break;
    case ButtonId::Theme: toggleTheme(); break;
    case ButtonId::Speed: cycleSpeed(); break;
    case ButtonId::StepBy: toggleStepBy(); break;
    case ButtonId::Code: togglePseudocode(); break;
    }
}

// ===================================================================== events

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

int Application::hitNode(sf::Vector2f local) const {
    int best = -1;
    float bestDistance = NodeRadius * 1.6f;
    for (const auto& node : graph_.nodes()) {
        const float d = distance(local, {node.x, node.y});
        if (d < bestDistance) {
            best = node.id;
            bestDistance = d;
        }
    }
    return best;
}

void Application::handleMousePressed(sf::Vector2i pixel, sf::Mouse::Button button) {
    const sf::Vector2f screen = window_.mapPixelToCoords(pixel, uiView_);
    if (button == sf::Mouse::Button::Left) {
        // The obstacle menu sits on top of everything else, so check it first.
        if (obstacleMenuOpen_ && !learningView_) {
            const std::array<std::pair<ObstacleTool, const char*>, 5> items{{
                {ObstacleTool::AddCircle, "Circle tool: click inside the graph"},
                {ObstacleTool::AddRectangle, "Rectangle tool: click inside the graph"},
                {ObstacleTool::Delete, "Delete tool: click an obstacle"},
                {ObstacleTool::SetStart, "Click a node to make it the start"},
                {ObstacleTool::SetGoal, "Click a node to make it the goal"},
            }};
            for (std::size_t index = 0; index < items.size(); ++index) {
                const sf::FloatRect item({794.f, 118.f + 34.f * static_cast<float>(index)}, {240.f, 34.f});
                if (item.contains(screen)) {
                    obstacleTool_ = items[index].first;
                    obstacleMenuOpen_ = false;
                    status_ = items[index].second;
                    return;
                }
            }
            obstacleMenuOpen_ = false;
        }
        for (const auto& candidate : buttons_) {
            if (!contains(candidate.bounds, screen)) continue;
            pressButton(candidate.id);
            return;
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

    if (local.x < 0.f || local.y < 0.f || local.x > CanvasWidth || local.y > CanvasHeight) return;

    if (awaiting_ && button == sf::Mouse::Button::Left) {
        const int node = hitNode(local);
        if (node >= 0) answer(node);
        return;
    }

    // Start and goal: menu tools, or Shift+click (start) and Ctrl/Cmd+click (goal).
    const bool shift = modifierDown(sf::Keyboard::Key::LShift, sf::Keyboard::Key::RShift);
    const bool command = modifierDown(sf::Keyboard::Key::LControl, sf::Keyboard::Key::RControl) ||
                         modifierDown(sf::Keyboard::Key::LSystem, sf::Keyboard::Key::RSystem);
    const bool settingStart = obstacleTool_ == ObstacleTool::SetStart || (shift && button == sf::Mouse::Button::Left);
    const bool settingGoal = obstacleTool_ == ObstacleTool::SetGoal || (command && button == sf::Mouse::Button::Left);
    if (settingStart || settingGoal) {
        const int node = hitNode(local);
        if (node < 0) {
            status_ = "Click on a node to choose it.";
            return;
        }
        if ((settingStart && node == goalNode_) || (settingGoal && node == startNode_)) {
            status_ = "Start and goal must be different nodes.";
            return;
        }
        (settingStart ? startNode_ : goalNode_) = node;
        obstacleTool_ = ObstacleTool::Move;
        invalidate(std::string(settingStart ? "Start" : "Goal") + " is now node " + std::to_string(node) +
                   ". Press Play or Step.");
        return;
    }

    if (button == sf::Mouse::Button::Right || obstacleTool_ == ObstacleTool::Delete) {
        deleteObstacleAt(local);
        return;
    }
    if (obstacleTool_ == ObstacleTool::AddCircle || obstacleTool_ == ObstacleTool::AddRectangle) {
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
    if (dragKind_ == DragKind::None) return;
    sf::Vector2f local = toLocal(pixel);
    local.x = std::clamp(local.x, 0.f, CanvasWidth);
    local.y = std::clamp(local.y, 0.f, CanvasHeight);

    if (dragKind_ == DragKind::Circle) circles_[draggedIndex_].center = local - dragOffset_;
    if (dragKind_ == DragKind::Rectangle) rectangles_[draggedIndex_].position = local - dragOffset_;
    movedOnce_ = true;
    recomputeEdgeAvailability();
    invalidate("The graph changed. Dashed red edges are blocked. Run the search again.");
}

void Application::handleKeyPressed(sf::Keyboard::Scancode key) {
    using Key = sf::Keyboard::Scancode;
    if (key == Key::Escape) {
        if (obstacleMenuOpen_) obstacleMenuOpen_ = false;
        else window_.close();
        return;
    }
    if (learningView_) {
        if (key == Key::Left) learningPage_ = std::max(0, learningPage_ - 1);
        if (key == Key::Right) learningPage_ = std::min(3, learningPage_ + 1);
        if (key == Key::L) toggleLearningView();
        if (key == Key::T) toggleTheme();
        return;
    }
    switch (key) {
    case Key::Space: pressButton(ButtonId::Play); break;
    case Key::Enter: pressButton(ButtonId::Play); break;
    case Key::Right: pressButton(ButtonId::Step); break;
    case Key::Left: stepBack(); break;
    case Key::Home: restartPlayback(); break;
    case Key::End: finishPlayback(); break;
    case Key::Num1: setStrategy(SearchStrategy::AStar); break;
    case Key::Num2: setStrategy(SearchStrategy::Dijkstra); break;
    case Key::Num3: setStrategy(SearchStrategy::Greedy); break;
    case Key::Num4: setStrategy(SearchStrategy::WeightedAStar); break;
    case Key::Q: togglePredict(); break;
    case Key::C: toggleChallenges(); break;
    case Key::N: toggleStepBy(); break;
    case Key::S: cycleSpeed(); break;
    case Key::R: resetScene(); break;
    case Key::G: randomizeRoadmap(); break;
    case Key::T: toggleTheme(); break;
    case Key::P: togglePseudocode(); break;
    case Key::L: toggleLearningView(); break;
    default: break;
    }
}

// ===================================================================== drawing

const char* Application::priorityName() const {
    switch (strategy_) {
    case SearchStrategy::Dijkstra: return "g";
    case SearchStrategy::Greedy: return "h";
    case SearchStrategy::WeightedAStar: return "f = g + 2h";
    default: return "f = g + h";
    }
}

std::string Application::scoreLabel(const NodeScore& score) const {
    switch (strategy_) {
    case SearchStrategy::Dijkstra: return "g=" + std::to_string(score.g);
    case SearchStrategy::Greedy: return "h=" + std::to_string(score.h);
    case SearchStrategy::WeightedAStar:
        return std::to_string(score.g) + "+2x" + std::to_string(score.h) + "=" + std::to_string(score.f);
    default:
        return std::to_string(score.g) + "+" + std::to_string(score.h) + "=" + std::to_string(score.f);
    }
}

std::string Application::buttonLabel(ButtonId id) const {
    switch (id) {
    case ButtonId::Play: return playing_ ? "Pause" : "Play";
    case ButtonId::Back: return "< Back";
    case ButtonId::Step: return awaiting_ ? "Guess!" : "Step >";
    case ButtonId::Finish: return "Finish";
    case ButtonId::Strategy: return std::string("Strategy: ") + strategyName(strategy_);
    case ButtonId::Predict: return predict_ ? "Predict: ON" : "Predict: off";
    case ButtonId::Obstacles: return "Edit graph";
    case ButtonId::Learn: return learningView_ ? "Back to lab" : "Learn A*";
    case ButtonId::Challenges: {
        int done = 0;
        for (const bool solved : challengesDone_) done += solved ? 1 : 0;
        return "Challenges " + std::to_string(done) + "/" + std::to_string(ChallengeCount);
    }
    case ButtonId::Reset: return "Reset";
    case ButtonId::Theme: return darkMode_ ? "Light" : "Dark";
    case ButtonId::Speed: return "Speed " + std::to_string(speed_);
    case ButtonId::StepBy: return stepByNode_ ? "Step: node" : "Step: line";
    case ButtonId::Code: return showPseudocode_ ? "Hide code" : "Show code";
    }
    return "";
}

bool Application::buttonActive(ButtonId id) const {
    switch (id) {
    case ButtonId::Play: return playing_;
    case ButtonId::Predict: return predict_;
    case ButtonId::Obstacles: return obstacleMenuOpen_ || obstacleTool_ != ObstacleTool::Move;
    case ButtonId::Learn: return learningView_;
    case ButtonId::Challenges: return showChallenges_;
    default: return false;
    }
}

bool Application::buttonEnabled(ButtonId id) const {
    if (learningView_) return id == ButtonId::Learn || id == ButtonId::Theme;
    switch (id) {
    case ButtonId::Back: return result_.has_value() && position_ >= 0;
    case ButtonId::Step: return !awaiting_ && !atEnd();
    case ButtonId::Finish: return !atEnd();
    default: return true;
    }
}

void Application::drawButton(const Button& button) {
    const bool active = buttonActive(button.id);
    const bool enabled = buttonEnabled(button.id);
    sf::Color accent = Expanded;
    if (button.id == ButtonId::Play) accent = playing_ ? Goal : Start;
    if (button.id == ButtonId::Predict || button.id == ButtonId::Challenges) accent = OpenRing;
    if (button.id == ButtonId::Obstacles) accent = ObstacleBorder;
    sf::RectangleShape shape(button.bounds.size);
    shape.setPosition(button.bounds.position);
    const bool filled = active || (button.id == ButtonId::Play && enabled);
    shape.setFillColor(button.utility ? PanelLight : (filled ? accent : Panel));
    shape.setOutlineThickness(button.utility ? 0.f : 1.f);
    shape.setOutlineColor(filled ? accent : Edge);
    window_.draw(shape);
    const sf::Color textColor = !enabled ? Edge : (filled && !button.utility ? Panel : (button.utility ? Muted : Text));
    drawText(buttonLabel(button.id),
             button.bounds.position + (button.utility ? sf::Vector2f{10.f, 7.f} : sf::Vector2f{12.f, 11.f}),
             button.utility ? 1.4f : 1.75f, textColor, true);
}

void Application::render() {
    window_.clear(Background);
    if (learningView_) {
        drawLearningView();
    } else {
        drawRoadmap();
        drawObstacles();
        drawSidebar();
        drawLegend();
    }
    drawHeader();
    window_.display();
}

void Application::drawHeader() {
    drawText("AStarLab", {42.f, 18.f}, 3.f, Text, true);
    drawText("See every decision behind the shortest path", {200.f, 27.f}, 1.7f, Muted);
    for (const auto& button : buttons_) {
        if (learningView_ && button.utility && button.id != ButtonId::Theme) continue;
        drawButton(button);
    }

    if (!learningView_) {
        const bool alert = awaiting_ || status_.rfind("Not node", 0) == 0;
        drawWrapped(status_, {42.f, 120.f}, 1.25f, alert ? OpenRing : Text, 100, 18.f);
    }

    if (obstacleMenuOpen_ && !learningView_) {
        const std::array<std::pair<const char*, sf::Color>, 5> items{{
            {"Add circle", Text}, {"Add rectangle", Text}, {"Delete obstacle", Goal},
            {"Set start node  (Shift+click)", Start}, {"Set goal node  (Ctrl/Cmd+click)", Goal},
        }};
        sf::RectangleShape menu({240.f, 34.f * static_cast<float>(items.size())});
        menu.setPosition({794.f, 118.f});
        menu.setFillColor(PanelLight);
        menu.setOutlineThickness(1.f);
        menu.setOutlineColor(Edge);
        window_.draw(menu);
        for (std::size_t index = 0; index < items.size(); ++index) {
            drawText(items[index].first, {806.f, 124.f + 34.f * static_cast<float>(index)}, 1.3f,
                     items[index].second);
        }
    }
}

void Application::drawRoadmap() {
    sf::RectangleShape canvas({CanvasWidth, CanvasHeight});
    canvas.setPosition({CanvasLeft, CanvasTop});
    canvas.setFillColor(Panel);
    canvas.setOutlineThickness(1.f);
    canvas.setOutlineColor(Edge);
    window_.draw(canvas);

    const bool finished = atEnd() && result_->found;
    const auto onPath = [&](int a, int b) {
        if (!finished) return false;
        const auto& path = result_->path;
        for (std::size_t index = 1; index < path.size(); ++index) {
            if ((path[index - 1] == a && path[index] == b) || (path[index - 1] == b && path[index] == a)) return true;
        }
        return false;
    };

    for (const auto& edge : graph_.edges()) {
        const sf::Vector2f from = toScreen(graph_.node(edge.from));
        const sf::Vector2f to = toScreen(graph_.node(edge.to));
        const bool inspecting = currentScore_ && currentNeighbor_ >= 0 &&
            ((edge.from == currentScore_->nodeId && edge.to == currentNeighbor_) ||
             (edge.to == currentScore_->nodeId && edge.from == currentNeighbor_));
        if (onPath(edge.from, edge.to)) {
            drawLine(from, to, 7.f, Path);
        } else if (inspecting) {
            drawLine(from, to, 5.f, Path);
        } else if (edge.enabled) {
            drawLine(from, to, 2.f, Edge);
        } else {
            // Dashed line for blocked edges.
            const sf::Vector2f delta = to - from;
            const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            const sf::Vector2f unit = delta / length;
            for (float t = 0.f; t < length; t += 11.f) {
                drawLine(from + unit * t, from + unit * std::min(length, t + 6.f), 1.6f, BlockedEdge);
            }
        }

        const sf::Vector2f midpoint = (from + to) / 2.f;
        sf::RectangleShape costBackground({24.f, 15.f});
        costBackground.setPosition(midpoint - sf::Vector2f{12.f, 8.f});
        costBackground.setFillColor(Panel);
        window_.draw(costBackground);
        const std::string cost = std::to_string(edge.cost);
        drawText(cost, midpoint - sf::Vector2f{static_cast<float>(cost.size()) * 3.3f, 5.f}, 1.1f,
                 edge.enabled ? Muted : BlockedEdge);
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
        if (!movedOnce_) {
            drawText("drag me", {CanvasLeft + obstacle.center.x - 24.f, CanvasTop + obstacle.center.y - 8.f},
                     1.3f, Text, true);
        }
    }
    for (const auto& obstacle : rectangles_) {
        sf::RectangleShape shape(obstacle.size);
        shape.setPosition({CanvasLeft + obstacle.position.x, CanvasTop + obstacle.position.y});
        shape.setFillColor(Obstacle);
        shape.setOutlineThickness(3.f);
        shape.setOutlineColor(ObstacleBorder);
        window_.draw(shape);
        if (!movedOnce_) {
            drawText("drag me", {CanvasLeft + obstacle.position.x + obstacle.size.x / 2.f - 24.f,
                                 CanvasTop + obstacle.position.y + obstacle.size.y / 2.f - 8.f},
                     1.3f, Text, true);
        }
    }
}

void Application::drawSidebar() {
    const float left = 982.f;
    if (showChallenges_) {
        drawChallengePanel(left, 180.f);
        return;
    }
    if (showPseudocode_) {
        drawPseudocode(left, 180.f);
        drawLiveValues(left, 524.f);
        drawOpenSetOrComparison(left, 628.f);
    } else {
        drawText("Visualization focus", {left, 180.f}, 2.1f, Text, true);
        drawText("Pseudocode is hidden (P to show)", {left, 214.f}, 1.15f, Muted);
        drawLiveValues(left, 250.f);
        drawOpenSetOrComparison(left, 360.f);
    }
}

void Application::drawPseudocode(float left, float top) {
    drawText(std::string(strategyName(strategy_)) + " pseudocode", {left, top}, 2.1f, Text, true);
    drawText("The active line follows the search", {left, top + 31.f}, 1.05f, Muted);

    struct CodeToken {
        std::string text;
        sf::Color color;
        bool bold{false};
    };
    struct CodeLine {
        int indent{};
        std::vector<CodeToken> tokens;
    };
    const auto token = [](std::string text, sf::Color color, bool bold = false) {
        return CodeToken{std::move(text), color, bold};
    };
    const std::array<CodeLine, 9> pseudocode{{
        {0, {token("openSet", SyntaxVariable), token(" = ", SyntaxOperator),
             token("PriorityQueue", SyntaxFunction), token("({ ", Text),
             token("start", SyntaxVariable), token(" })", Text)}},
        {0, {token("while not", SyntaxKeyword, true), token(" openSet.", Text),
             token("empty", SyntaxFunction), token("():", Text)}},
        {1, {token("current", SyntaxVariable), token(" = ", SyntaxOperator),
             token("openSet.", Text), token(popName(strategy_), SyntaxFunction), token("()", Text)}},
        {1, {token("if", SyntaxKeyword, true), token(" current ", SyntaxVariable),
             token("==", SyntaxOperator), token(" goal:", SyntaxVariable)}},
        {2, {token("return", SyntaxKeyword, true), token(" reconstructPath", SyntaxFunction),
             token("(current)", SyntaxVariable)}},
        {1, {token("for", SyntaxKeyword, true), token(" neighbor ", SyntaxVariable),
             token("in", SyntaxKeyword, true), token(" neighbors", SyntaxFunction),
             token("(current):", SyntaxVariable)}},
        {2, {token("tentativeG", SyntaxVariable), token(" = ", SyntaxOperator),
             token("g[current] ", SyntaxVariable), token("+", SyntaxOperator),
             token(" cost", SyntaxFunction), token("(current, neighbor)", SyntaxVariable)}},
        {2, {token("if", SyntaxKeyword, true), token(" tentativeG ", SyntaxVariable),
             token("<", SyntaxOperator), token(" g[neighbor]:", SyntaxVariable)}},
        {3, {token("updateScoresAndParent", SyntaxFunction),
             token("(neighbor, tentativeG)", SyntaxVariable)}},
    }};
    float lineTop = top + 57.f;
    for (std::size_t index = 0; index < pseudocode.size(); ++index) {
        const bool active = activePseudocodeLine_ == static_cast<int>(index + 1);
        if (active) {
            sf::RectangleShape highlight({258.f, 28.f});
            highlight.setPosition({left, lineTop - 4.f});
            highlight.setFillColor({Expanded.r, Expanded.g, Expanded.b, 76});
            window_.draw(highlight);
        }

        sf::RectangleShape gutter({1.f, 28.f});
        gutter.setPosition({left + 30.f, lineTop - 4.f});
        gutter.setFillColor(Edge);
        window_.draw(gutter);
        drawText(std::to_string(index + 1), {left + 12.f, lineTop}, 1.05f,
                 active ? Text : Edge, active);

        constexpr float HorizontalScale = 0.72f;
        float tokenLeft = left + 40.f + static_cast<float>(pseudocode[index].indent) * 12.f;
        for (const auto& codeToken : pseudocode[index].tokens) {
            sf::Text label(font_, codeToken.text, 10);
            label.setPosition({tokenLeft, lineTop});
            label.setFillColor(codeToken.color);
            label.setScale({HorizontalScale, 1.f});
            if (codeToken.bold) label.setStyle(sf::Text::Bold);
            window_.draw(label);
            const auto& glyphs = label.getShapedGlyphs();
            if (!glyphs.empty()) {
                const auto& lastGlyph = glyphs.back();
                tokenLeft += (lastGlyph.position.x + lastGlyph.glyph.advance) * HorizontalScale;
            }
        }
        lineTop += 31.f;
    }
}

void Application::drawLiveValues(float left, float top) {
    sf::RectangleShape box({258.f, 96.f});
    box.setPosition({left, top});
    box.setFillColor(PanelLight);
    box.setOutlineThickness(1.f);
    box.setOutlineColor(currentScore_ ? Expanded : Edge);
    window_.draw(box);
    drawText("Right now", {left + 12.f, top + 8.f}, 1.15f, Muted, true);
    if (!currentScore_) {
        drawText("Press Step to begin", {left + 12.f, top + 34.f}, 1.25f, Muted);
        return;
    }
    std::string nodes = "Current " + std::to_string(currentScore_->nodeId);
    if (currentNeighbor_ >= 0) nodes += "   Neighbor " + std::to_string(currentNeighbor_);
    drawText(nodes, {left + 12.f, top + 28.f}, 1.2f, Text, true);
    drawText("g " + std::to_string(currentScore_->g), {left + 12.f, top + 50.f}, 1.45f, Start, true);
    drawText("h " + std::to_string(currentScore_->h), {left + 92.f, top + 50.f}, 1.45f, Goal, true);
    drawText("f " + std::to_string(currentScore_->f), {left + 172.f, top + 50.f}, 1.45f, Path, true);
    if (lastStep_ && (lastStep_->type == SearchStepType::CalculateTentative ||
                      lastStep_->type == SearchStepType::CompareCost)) {
        std::string calc = lastStep_->type == SearchStepType::CalculateTentative
            ? "tentativeG = " + std::to_string(lastStep_->score.g) + " + " + std::to_string(lastStep_->edgeCost) +
                  " = " + std::to_string(lastStep_->tentativeG)
            : std::to_string(lastStep_->tentativeG) + " < " + infinityOr(lastStep_->previousG) + " ?  " +
                  (lastStep_->improved ? "yes, update" : "no, keep");
        drawText(calc, {left + 12.f, top + 74.f}, 1.1f, Muted);
    }
}

void Application::drawOpenSetOrComparison(float left, float top) {
    sf::RectangleShape box({258.f, 140.f});
    box.setPosition({left, top});
    box.setFillColor(PanelLight);
    window_.draw(box);
    if (atEnd() && !comparison_.empty()) {
        drawText("Same graph, every strategy", {left + 12.f, top + 8.f}, 1.15f, Muted, true);
        drawText("Strategy", {left + 12.f, top + 30.f}, 1.05f, Muted);
        drawText("Cost", {left + 150.f, top + 30.f}, 1.05f, Muted);
        drawText("Closed", {left + 200.f, top + 30.f}, 1.05f, Muted);
        int best = 1 << 30;
        for (const auto& row : comparison_) if (row.found) best = std::min(best, row.cost);
        float rowTop = top + 50.f;
        for (const auto& row : comparison_) {
            const sf::Color color = row.strategy == strategy_ ? OpenRing : Text;
            drawText(strategyName(row.strategy), {left + 12.f, rowTop}, 1.2f, color, row.strategy == strategy_);
            drawText(row.found ? std::to_string(row.cost) + (row.cost == best ? " ok" : "") : "none",
                     {left + 150.f, rowTop}, 1.2f, row.found && row.cost == best ? Start : color);
            drawText(std::to_string(row.closed), {left + 206.f, rowTop}, 1.2f, color);
            rowTop += 21.f;
        }
        return;
    }
    drawText(std::string("Open set (by ") + (strategy_ == SearchStrategy::Dijkstra ? "g" : strategy_ == SearchStrategy::Greedy ? "h" : "f") +
                 (awaiting_ ? ", by node no.)" : ", next first)"),
             {left + 12.f, top + 8.f}, 1.15f, Muted, true);
    auto entries = sortedOpenSet();
    if (awaiting_) {
        // Don't give the answer away while the student is predicting.
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    }
    if (entries.empty()) {
        drawText("empty", {left + 12.f, top + 34.f}, 1.2f, Muted);
    }
    float chipLeft = left + 12.f;
    float chipTop = top + 32.f;
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const std::string text = std::to_string(entries[index].first) + " : " + std::to_string(entries[index].second.score.f);
        const float width = 16.f + static_cast<float>(text.size()) * 7.f;
        if (chipLeft + width > left + 250.f) {
            chipLeft = left + 12.f;
            chipTop += 24.f;
        }
        if (chipTop > top + 110.f) {
            drawText("...", {chipLeft, chipTop}, 1.2f, Muted);
            break;
        }
        sf::RectangleShape chip({width, 20.f});
        chip.setPosition({chipLeft, chipTop});
        const bool next = index == 0 && !awaiting_;
        chip.setFillColor(next ? sf::Color(OpenRing.r, OpenRing.g, OpenRing.b, 60) : PanelLight);
        chip.setOutlineThickness(1.5f);
        chip.setOutlineColor(OpenRing);
        window_.draw(chip);
        drawText(text, {chipLeft + 8.f, chipTop + 2.f}, 1.1f, Text, next);
        chipLeft += width + 6.f;
    }
    drawText(std::to_string(closedNodes_.size()) + " nodes closed", {left + 12.f, top + 116.f}, 1.05f, Muted);
}

void Application::drawChallengePanel(float left, float top) {
    drawText("Challenges", {left, top}, 2.1f, Text, true);
    drawWrapped("Change the graph until a challenge turns green. Checked automatically.", {left, top + 32.f},
                1.05f, Muted, 44, 15.f);
    float cardTop = top + 70.f;
    for (int index = 0; index < ChallengeCount; ++index) {
        const auto& text = Challenges[static_cast<std::size_t>(index)];
        const auto& status = challengeStatus_[static_cast<std::size_t>(index)];
        const bool done = challengesDone_[static_cast<std::size_t>(index)];
        sf::RectangleShape card({258.f, 100.f});
        card.setPosition({left, cardTop});
        card.setFillColor(PanelLight);
        card.setOutlineThickness(status.solved ? 2.f : 1.f);
        card.setOutlineColor(status.solved ? Start : Edge);
        window_.draw(card);
        drawText(std::string(done ? "[x] " : "[ ] ") + text.title, {left + 10.f, cardTop + 7.f}, 1.25f,
                 done ? Start : Text, true);
        drawWrapped(text.task, {left + 10.f, cardTop + 30.f}, 1.05f, Muted, 42, 15.f);
        drawText("Now: " + status.note, {left + 10.f, cardTop + 78.f}, 1.0f, status.solved ? Start : OpenRing);
        cardTop += 108.f;
    }
}

void Application::drawLegend() {
    const float top = 780.f;
    const std::array<std::pair<sf::Color, std::string>, 6> items{{
        {Start, "Start"}, {Goal, "Goal"}, {OpenRing, "Open set"}, {Expanded, "Closed"},
        {Path, "Best path"}, {ObstacleBorder, "Obstacle"},
    }};
    float left = CanvasLeft;
    for (const auto& [color, label] : items) {
        sf::CircleShape dot(6.f);
        dot.setPosition({left, top});
        dot.setFillColor(color);
        window_.draw(dot);
        drawText(label, {left + 19.f, top + 1.f}, 1.3f, Muted);
        left += static_cast<float>(label.size()) * 8.f + 52.f;
    }
    drawText("Space play/pause   Right/Left step   Home/End   1-4 strategy   Q predict   C challenges   "
             "N step size   S speed   R reset   G random   T theme   P code   L learn",
             {CanvasLeft, 814.f}, 1.1f, Muted);
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
            "Step through the search (Right arrow) and watch the open set in the sidebar.",
            "Turn on Predict (Q): click the open node you think comes out next.",
            "Switch strategy (1-4): which still find the cheapest path? Which close fewer nodes?",
            "Move the start (Shift+click) and the goal (Ctrl/Cmd+click) and run again.",
            "Open Challenges (C) and change the graph until each one turns green.",
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
    const auto open = openSet_.find(nodeId);
    const bool isOpen = open != openSet_.end();
    const bool isClosed = closedNodes_.count(nodeId) != 0U;
    const bool onPath = atEnd() && result_->found &&
        std::find(result_->path.begin(), result_->path.end(), nodeId) != result_->path.end();
    const bool isEndpoint = nodeId == startNode_ || nodeId == goalNode_;

    sf::Color color = Node;
    if (isClosed) color = Expanded;
    if (onPath) color = Path;
    if (nodeId == startNode_) color = Start;
    if (nodeId == goalNode_) color = Goal;

    if (isOpen && awaiting_) {
        sf::CircleShape glow(NodeRadius + 9.f);
        glow.setOrigin({NodeRadius + 9.f, NodeRadius + 9.f});
        glow.setPosition(position);
        glow.setFillColor({OpenRing.r, OpenRing.g, OpenRing.b, 70});
        window_.draw(glow);
    }

    sf::CircleShape circle(NodeRadius);
    circle.setOrigin({NodeRadius, NodeRadius});
    circle.setPosition(position);
    circle.setFillColor(color);
    const bool isCurrent = currentScore_ && currentScore_->nodeId == nodeId && !awaiting_;
    const bool isNeighbor = currentNeighbor_ == nodeId;
    if (isOpen && !isEndpoint) {
        circle.setOutlineThickness(3.f);
        circle.setOutlineColor(OpenRing);
    } else {
        circle.setOutlineThickness(isCurrent || isNeighbor ? 4.f : 2.f);
        circle.setOutlineColor(isCurrent ? Text : (isNeighbor ? Path : Edge));
    }
    window_.draw(circle);
    if (isCurrent || isNeighbor) {
        sf::CircleShape ring(NodeRadius + 5.f);
        ring.setOrigin({NodeRadius + 5.f, NodeRadius + 5.f});
        ring.setPosition(position);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(2.5f);
        ring.setOutlineColor(isCurrent ? Text : Path);
        window_.draw(ring);
    }

    const std::string label = std::to_string(nodeId);
    drawText(label, {position.x - static_cast<float>(label.size()) * 3.6f, position.y - 8.f}, 1.2f, Text, true);

    std::optional<NodeScore> score;
    if (isOpen) score = open->second.score;
    else if (const auto closed = closedScores_.find(nodeId); closed != closedScores_.end()) score = closed->second;
    if (score) {
        const std::string values = scoreLabel(*score);
        drawText(values, {position.x - static_cast<float>(values.size()) * 2.9f, position.y + 21.f}, 1.05f,
                 isOpen ? OpenRing : Muted);
    }
}

float Application::drawWrapped(const std::string& text, sf::Vector2f position, float pixelSize,
                               sf::Color color, std::size_t maxChars, float lineHeight, bool bold) {
    std::istringstream words(text);
    std::string word;
    std::string line;
    float y = position.y;
    const auto flush = [&]() {
        if (line.empty()) return;
        drawText(line, {position.x, y}, pixelSize, color, bold);
        y += lineHeight;
        line.clear();
    };
    while (words >> word) {
        if (!line.empty() && line.size() + 1 + word.size() > maxChars) flush();
        line += (line.empty() ? "" : " ") + word;
    }
    flush();
    return y - position.y;
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

