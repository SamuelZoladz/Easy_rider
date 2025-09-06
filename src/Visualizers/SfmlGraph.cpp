#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>

void SfmlSimulationVisualizer::rebuildGraphCache() {
  if (!graphProvider_)
    return;

  const auto data = graphProvider_();

  edgesVA_ = std::make_unique<sf::VertexArray>(sf::Triangles);
  const float halfT = std::max(0.f, edgeThickness_) * 0.5f;

  for (const auto &e : data.edges) {
    const sf::Vector2f a = toScreen(data.nodePositions.at(e.first), view_);
    const sf::Vector2f b = toScreen(data.nodePositions.at(e.second), view_);

    // wektor kierunkowy
    sf::Vector2f d(b.x - a.x, b.y - a.y);
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 1e-4f) {
      // zbyt krótka krawędź — pomiń
      continue;
    }

    // wektor prostopadły znormalizowany (ekranowe piksele)
    sf::Vector2f n(-d.y / len * halfT, d.x / len * halfT);

    // cztery wierzchołki prostokąta
    const sf::Vector2f v0 = sf::Vector2f(a.x + n.x, a.y + n.y);
    const sf::Vector2f v1 = sf::Vector2f(b.x + n.x, b.y + n.y);
    const sf::Vector2f v2 = sf::Vector2f(b.x - n.x, b.y - n.y);
    const sf::Vector2f v3 = sf::Vector2f(a.x - n.x, a.y - n.y);

    // dwa trójkąty tworzące „grubą” linię
    edgesVA_->append(sf::Vertex(v0));
    edgesVA_->append(sf::Vertex(v1));
    edgesVA_->append(sf::Vertex(v2));

    edgesVA_->append(sf::Vertex(v2));
    edgesVA_->append(sf::Vertex(v3));
    edgesVA_->append(sf::Vertex(v0));
  }

  // === WĘZŁY: bez zmian ===
  nodesVA_ = std::make_unique<sf::VertexArray>(sf::Points);
  nodesVA_->clear();
  for (const auto &p : data.nodePositions) {
    nodesVA_->append(sf::Vertex(toScreen(p, view_)));
  }

  graphCacheDirty_ = false;
}
void SfmlSimulationVisualizer::drawGraph(sf::RenderTarget &target) {
  if (graphCacheDirty_)
    rebuildGraphCache();
  if (edgesVA_)
    target.draw(*edgesVA_);
  if (nodesVA_) {
    sf::CircleShape node(nodeRadius_);
    node.setOrigin(nodeRadius_, nodeRadius_);
    for (std::size_t i = 0; i < nodesVA_->getVertexCount(); ++i) {
      node.setPosition(nodesVA_->operator[](i).position);
      target.draw(node);
    }
  }
}

void SfmlSimulationVisualizer::drawVehicles(sf::RenderTarget &target) {
  if (!vehicleProvider_)
    return;
  const auto vehicles = vehicleProvider_();
  sf::CircleShape dot(vehicleRadius_);
  dot.setOrigin(vehicleRadius_, vehicleRadius_);
  for (const auto &wpos : vehicles) {
    dot.setPosition(toScreen(wpos, view_));
    target.draw(dot);
  }
}
