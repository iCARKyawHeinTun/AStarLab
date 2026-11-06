#pragma once

#include "AStar.hpp"

#include <SFML/Graphics.hpp>

#include <array>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Application {
public:
    Application();
    void run();

private:
    enum class DragKind { None, Circle, Rectangle };
    enum class ObstacleTool { Move, AddCircle, AddRectangle, Delete, SetStart, SetGoal };
    enum class ButtonId {
        Play, Back, Step, Finish, Strategy, Predict, Obstacles, Learn, Challenges,
        Reset, Theme, Speed, StepBy, Code,
    };

    struct CircleObstacle {
        sf::Vector2f center;
        float radius;
    };

    struct RectangleObstacle {
        sf::Vector2f position;
        sf::Vector2f size;
    };

    struct Button {
        ButtonId id;
        sf::FloatRect bounds;
        bool utility; ///< small secondary control in the second row
    };

    struct OpenEntry {
        NodeScore score;
        int sequence{};
    };

    struct ComparisonRow {
        SearchStrategy strategy;
        bool found{};
        int cost{};
        int closed{};
    };

    struct ChallengeStatus {
        bool solved{};
        std::string note;
    };

    // ---- scene
    void buildRoadmap(bool randomized = false);
    void randomizeRoadmap();
    void resetScene();
    void recomputeEdgeAvailability();
    void addObstacleAt(sf::Vector2f local);
    bool deleteObstacleAt(sf::Vector2f local);
    int obstacleCount() const { return static_cast<int>(circles_.size() + rectangles_.size()); }

    // ---- search playback (state on screen = replay of steps 0..position_)
    void invalidate(const std::string& status);
    void ensureResult();
    void derive();
    bool atEnd() const;
    bool stepForward();
    void stepBack();
    void restartPlayback();
    void finishPlayback();
    void play();
    void pause();
    void present();
    std::string describeStep(const SearchStep& step) const;
    std::vector<std::pair<int, OpenEntry>> sortedOpenSet() const;
    void computeComparison();

    // ---- predict mode
    void ask();
    void answer(int nodeId);
    void finishAnswer(const std::string& message);

    // ---- challenges
    void evaluateChallenges();

    // ---- toggles
    void toggleTheme();
    void cycleSpeed();
    void toggleStepBy();
    void togglePseudocode();
    void toggleLearningView();
    void togglePredict();
    void toggleChallenges();
    void setStrategy(SearchStrategy strategy);
    void cycleStrategy();
    void pressButton(ButtonId id);

    // ---- events
    void processEvents();
    void updateResponsiveView(sf::Vector2u windowSize);
    void handleMousePressed(sf::Vector2i pixel, sf::Mouse::Button button);
    void handleMouseMoved(sf::Vector2i pixel);
    void handleKeyPressed(sf::Keyboard::Scancode key);
    void update(sf::Time elapsed);
    int hitNode(sf::Vector2f local) const;

    // ---- drawing
    void render();
    void drawHeader();
    void drawButton(const Button& button);
    std::string buttonLabel(ButtonId id) const;
    bool buttonActive(ButtonId id) const;
    bool buttonEnabled(ButtonId id) const;
    void drawRoadmap();
    void drawObstacles();
    void drawSidebar();
    void drawPseudocode(float left, float top);
    void drawLiveValues(float left, float top);
    void drawOpenSetOrComparison(float left, float top);
    void drawChallengePanel(float left, float top);
    void drawLegend();
    void drawLearningView();
    void drawNode(int nodeId);
    void drawLine(sf::Vector2f from, sf::Vector2f to, float thickness, sf::Color color);
    void drawText(const std::string& text, sf::Vector2f position, float pixelSize,
                  sf::Color color, bool bold = false);
    float drawWrapped(const std::string& text, sf::Vector2f position, float pixelSize,
                      sf::Color color, std::size_t maxChars, float lineHeight, bool bold = false);
    std::string scoreLabel(const NodeScore& score) const;
    const char* priorityName() const;

    sf::Vector2f toScreen(const GraphNode& node) const;
    sf::Vector2f toLocal(sf::Vector2i pixel) const;
    bool edgeIntersectsObstacle(const GraphEdge& edge) const;
    static bool segmentIntersectsCircle(sf::Vector2f start, sf::Vector2f end,
                                        const CircleObstacle& circle);
    static bool segmentIntersectsRectangle(sf::Vector2f start, sf::Vector2f end,
                                           const RectangleObstacle& rectangle);
    static bool contains(const sf::FloatRect& bounds, sf::Vector2f point);

    static constexpr unsigned WindowWidth = 1280;
    static constexpr unsigned WindowHeight = 860;
    static constexpr float CanvasLeft = 42.f;
    static constexpr float CanvasTop = 176.f;
    static constexpr float CanvasWidth = 910.f;
    static constexpr float CanvasHeight = 570.f;
    static constexpr float NodeRadius = 15.f;
    static constexpr int ChallengeCount = 5;

    sf::RenderWindow window_;
    sf::View uiView_;
    sf::Font font_;
    Graph graph_;
    AStar astar_;
    int startNode_{0};
    int goalNode_{22};
    std::vector<CircleObstacle> circles_;
    std::vector<RectangleObstacle> rectangles_;
    std::vector<CircleObstacle> initialCircles_;
    std::vector<RectangleObstacle> initialRectangles_;

    // Search and playback
    SearchStrategy strategy_{SearchStrategy::AStar};
    std::optional<SearchResult> result_;
    int position_{-1};
    bool playing_{false};
    sf::Time playAccumulator_{sf::Time::Zero};
    int speed_{3};
    bool stepByNode_{false};
    std::vector<ComparisonRow> comparison_;

    // Derived by replaying steps 0..position_
    std::unordered_map<int, OpenEntry> openSet_;
    std::unordered_set<int> closedNodes_;
    std::unordered_map<int, NodeScore> closedScores_;
    std::optional<NodeScore> currentScore_;
    int currentNeighbor_{-1};
    int activePseudocodeLine_{-1};
    std::optional<SearchStep> lastStep_;

    // Predict mode
    bool predict_{false};
    bool awaiting_{false};
    bool answered_{false};
    int tries_{0};
    int predictRight_{0};
    int predictTotal_{0};
    bool resumeAfterAnswer_{false};

    // Challenges
    bool showChallenges_{false};
    std::array<bool, ChallengeCount> challengesDone_{};
    std::array<ChallengeStatus, ChallengeCount> challengeStatus_{};

    // Interface
    DragKind dragKind_{DragKind::None};
    std::size_t draggedIndex_{0};
    sf::Vector2f dragOffset_{};
    bool movedOnce_{false};
    std::mt19937 randomEngine_;
    std::vector<Button> buttons_;
    bool darkMode_{true};
    bool showPseudocode_{true};
    bool learningView_{false};
    bool obstacleMenuOpen_{false};
    int learningPage_{0};
    ObstacleTool obstacleTool_{ObstacleTool::Move};
    std::string status_{"Ready. Press Play, or Step through the search one line at a time."};
};
