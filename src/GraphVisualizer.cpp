#include "Easy_rider/GraphVisualizer.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <unordered_set>

GraphVisualizer::GraphVisualizer(unsigned int width, unsigned int height,
                                 const std::string &title)
    : window_(sf::VideoMode(width, height), title) {
  if (!font_.loadFromFile("assets/fonts/arial.ttf")) {
    std::cerr << "Warning: could not load font arial.ttf\n";
  }
}

void GraphVisualizer::processEvents() {
  sf::Event event;
  while (window_.pollEvent(event)) {
    if (event.type == sf::Event::Closed)
      window_.close();
  }
}

void GraphVisualizer::run(const Graph<Intersection, Road> &graph,
                          const std::vector<int> &pathIds1,
                          const std::vector<int> &pathIds2) {
  while (window_.isOpen()) {
    processEvents();
    window_.clear(sf::Color::White);
    draw(graph, pathIds1, pathIds2);
    window_.display();
  }
}

void GraphVisualizer::draw(const Graph<Intersection, Road> &graph,
                           const std::vector<int> &pathIds1,
                           const std::vector<int> &pathIds2) {
  int minSpeed = std::numeric_limits<int>::max();
  int maxSpeed = std::numeric_limits<int>::min();
  for (auto const &e : graph.getEdges()) {
    int s = e.getMaxSpeed();
    minSpeed = std::min(minSpeed, s);
    maxSpeed = std::max(maxSpeed, s);
  }
  int speedRange = std::max(1, maxSpeed - minSpeed);

  const float t_min = 1.f;
  const float t_max = 6.f;

  auto makeKey = [](int a, int b) -> std::uint64_t {
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a)) << 32) |
           static_cast<std::uint32_t>(b);
  };

  std::unordered_set<std::uint64_t> pathEdgeSet1;
  for (size_t i = 0; i + 1 < pathIds1.size(); ++i) {
    pathEdgeSet1.insert(makeKey(pathIds1[i], pathIds1[i + 1]));
    pathEdgeSet1.insert(makeKey(pathIds1[i + 1], pathIds1[i]));
  }
  std::unordered_set<std::uint64_t> pathEdgeSet2;
  for (size_t i = 0; i + 1 < pathIds2.size(); ++i) {
    pathEdgeSet2.insert(makeKey(pathIds2[i], pathIds2[i + 1]));
    pathEdgeSet2.insert(makeKey(pathIds2[i + 1], pathIds2[i]));
  }

  for (auto const &edge : graph.getEdges()) {
    auto [x1, y1] = graph.positionOf(edge.getFromId());
    auto [x2, y2] = graph.positionOf(edge.getToId());

    float speed = float(edge.getMaxSpeed() - minSpeed);
    float norm = speed / float(speedRange);
    float thickness = t_min + norm * (t_max - t_min);

    sf::Vector2f p1{float(x1), float(y1)};
    sf::Vector2f p2{float(x2), float(y2)};
    sf::Vector2f diff = p2 - p1;
    float length = std::hypot(diff.x, diff.y);

    sf::RectangleShape line(sf::Vector2f(length, thickness));

    bool onPath1 =
        pathEdgeSet1.count(makeKey(edge.getFromId(), edge.getToId())) > 0;
    bool onPath2 =
        pathEdgeSet2.count(makeKey(edge.getFromId(), edge.getToId())) > 0;

    if (onPath1 && onPath2) {
      line.setFillColor(sf::Color::Red);
    } else if (onPath1) {
      line.setFillColor(sf::Color::Green);
    } else if (onPath2) {
      line.setFillColor(sf::Color::Cyan);
    } else {
      line.setFillColor(sf::Color::Black);
    }

    line.setOrigin(0.f, thickness * 0.5f);
    line.setPosition(p1);
    float angle = std::atan2(diff.y, diff.x) * 180.f / 3.14159265f;
    line.setRotation(angle);

    window_.draw(line);
  }
  sf::Text label;
  label.setFont(font_);
  label.setCharacterSize(14);
  label.setFillColor(sf::Color::Black);
  label.setOutlineColor(sf::Color::White);
  label.setOutlineThickness(2.f);
  for (auto const &node : graph.getNodes()) {
    auto [x, y] = node.getPosition();
    sf::CircleShape circle(6.f);
    circle.setFillColor(sf::Color::Red);
    circle.setPosition(static_cast<float>(x) - 6.f,
                       static_cast<float>(y) - 6.f);
    window_.draw(circle);
    label.setString(std::to_string(node.getId()));
    label.setPosition(static_cast<float>(x) + 8.f,
                      static_cast<float>(y) - 10.f);
    window_.draw(label);
  }
}
