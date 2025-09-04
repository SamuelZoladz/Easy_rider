#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>

void SfmlSimulationVisualizer::rebuildGraphCache() {
  if (!graphProvider_)
    return;

  const auto data = graphProvider_();

  edgesVA_ = std::make_unique<sf::VertexArray>(sf::Lines);
  edgesVA_->clear();
  for (const auto &e : data.edges) {
    const viz::Vec2 &a = data.nodePositions.at(e.first);
    const viz::Vec2 &b = data.nodePositions.at(e.second);
    edgesVA_->append(sf::Vertex(toScreen(a, view_)));
    edgesVA_->append(sf::Vertex(toScreen(b, view_)));
  }

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
