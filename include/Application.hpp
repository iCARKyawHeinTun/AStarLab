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
    enum class AnimationPhase { Idle, Expanding, ShowingPath, Complete };
    enum class DragKind { None, Circle, Rectangle };
    enum class ObstacleTool { Move, AddCircle, AddRectangle, Delete };

    struct CircleObstacle {
        sf::Vector2f center;
        float radius;
    };

    struct RectangleObstacle {
        sf::Vector2f position;
        sf::Vector2f size;
    };

    struct Button {
        sf::FloatRect bounds;
        std::string label;
        sf::Color accent;
    };

    void buildRoadmap(bool randomized = false);
    void randomizeRoadmap();
    void resetScene();
    void recomputeEdgeAvailability();
    void clearSearch(const std::string& status);
    void startSearch();
    void cancelSearch();
    bool isSimulationRunning() const;
    void toggleTheme();
    void toggleSpeed();
    void togglePseudocode();
    void toggleLearningView();
    void addObstacleAt(sf::Vector2f local);
    bool deleteObstacleAt(sf::Vector2f local);

    void processEvents();
    void updateResponsiveView(sf::Vector2u windowSize);
    void handleMousePressed(sf::Vector2i pixel, sf::Mouse::Button button);
    void handleMouseMoved(sf::Vector2i pixel);
    void handleKeyPressed(sf::Keyboard::Scancode key);
    void update(sf::Time elapsed);

    void render();
    void drawHeader();
    void drawRoadmap();
    void drawObstacles();
    void drawSidebar();
    void applySearchStep(const SearchStep& step);
    void drawLegend();
    void drawLearningView();
    void drawNode(int nodeId);
    void drawLine(sf::Vector2f from, sf::Vector2f to, float thickness, sf::Color color);
    void drawText(const std::string& text, sf::Vector2f position, float pixelSize,
                  sf::Color color, bool bold = false);

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

    SearchResult result_;
    std::unordered_set<int> expandedNodes_;
    std::unordered_set<int> visiblePathNodes_;
    std::unordered_map<int, NodeScore> scores_;
    std::optional<NodeScore> currentScore_;
    int currentNeighbor_{-1};
    int activePseudocodeLine_{-1};
    std::size_t animationIndex_{0};
    sf::Time animationAccumulator_{sf::Time::Zero};
    AnimationPhase animationPhase_{AnimationPhase::Idle};

    DragKind dragKind_{DragKind::None};
    std::size_t draggedIndex_{0};
    sf::Vector2f dragOffset_{};
    std::mt19937 randomEngine_;
    std::array<Button, 4> buttons_;
    bool darkMode_{true};
    bool slowMode_{true};
    bool showPseudocode_{true};
    bool learningView_{false};
    bool obstacleMenuOpen_{false};
    int learningPage_{0};
    ObstacleTool obstacleTool_{ObstacleTool::Move};
    std::string status_{"Ready — move an obstacle or run the search"};
};

