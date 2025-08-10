#include "Easy_rider/GraphVisualizer.h"
#include <cmath>
#include <iostream>

GraphVisualizer::GraphVisualizer(unsigned int width, unsigned int height,
                                 const std::string &title)
    : window_(sf::VideoMode(width, height), title) {
  if (!font_.loadFromFile("assets/fonts/arial.ttf")) {
    std::cerr << "Warning: could not load font arial.ttf\n";
  }
}

void GraphVisualizer::run(const Graph<Intersection, Road> &graph) {
  while (window_.isOpen()) {
    processEvents();
    window_.clear(sf::Color::White);
    draw(graph);
    window_.display();
  }
}

void GraphVisualizer::processEvents() {
  sf::Event event;
  while (window_.pollEvent(event)) {
    if (event.type == sf::Event::Closed)
      window_.close();
  }
}

void GraphVisualizer::draw(const Graph<Intersection, Road> &graph) {
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
    line.setFillColor(sf::Color::Black);
    line.setOrigin(0.f, thickness * 0.5f);
    line.setPosition(p1);
    float angle = std::atan2(diff.y, diff.x) * 180.f / 3.14159265f;
    line.setRotation(angle);

    window_.draw(line);
  }

  for (auto const &node : graph.getNodes()) {
    auto [x, y] = node.getPosition();
    sf::CircleShape circle(6.f);
    circle.setFillColor(sf::Color::Red);
    circle.setPosition(static_cast<float>(x) - 6.f,
                       static_cast<float>(y) - 6.f);
    window_.draw(circle);
  }
}
