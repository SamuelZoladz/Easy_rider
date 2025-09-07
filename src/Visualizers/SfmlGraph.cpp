#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>

#include "Easy_rider/Parameters/Parameters.h"

void SfmlSimulationVisualizer::rebuildGraphCache() {
  const auto data = makeGraphDrawData(simulation_->graph());
  const float halfTWorld = std::max(0.1f, Parameters::edgeThickness()) * 0.5f;

  edgesVA_ = std::make_unique<sf::VertexArray>(sf::Triangles);

  // Build quads (two triangles) for each edge.
  for (const auto &e : data.edges) {
    const sf::Vector2f a = data.nodePositions.at(e.first);
    const sf::Vector2f b = data.nodePositions.at(e.second);

    sf::Vector2f d(b.x - a.x, b.y - a.y);
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 1e-4f)
      continue;

    const sf::Vector2f n(-d.y / len * halfTWorld, d.x / len * halfTWorld);

    const sf::Vector2f v0 = sf::Vector2f(a.x + n.x, a.y + n.y);
    const sf::Vector2f v1 = sf::Vector2f(b.x + n.x, b.y + n.y);
    const sf::Vector2f v2 = sf::Vector2f(b.x - n.x, b.y - n.y);
    const sf::Vector2f v3 = sf::Vector2f(a.x - n.x, a.y - n.y);

    edgesVA_->append(sf::Vertex(v0));
    edgesVA_->append(sf::Vertex(v1));
    edgesVA_->append(sf::Vertex(v2));
    edgesVA_->append(sf::Vertex(v2));
    edgesVA_->append(sf::Vertex(v3));
    edgesVA_->append(sf::Vertex(v0));
  }

  nodesVA_.clear();
  nodesVA_.reserve(data.nodePositions.size());
  for (const sf::Vector2f &p : data.nodePositions) {
    nodesVA_.push_back(p);
  }

  graphCacheDirty_ = false;
}

void SfmlSimulationVisualizer::drawVehicles(sf::RenderTarget &target) const {
  const auto vehicles = extractVehiclePositions(*simulation_);
  const float rW = std::max(0.1f, Parameters::vechicleRadius());

  sf::CircleShape dot(rW);
  dot.setOrigin(rW, rW);

  const auto old = target.getView();
  target.setView(sceneView_);
  for (const auto &wpos : vehicles) {
    dot.setPosition(wpos);
    target.draw(dot);
  }
  target.setView(old);
}

void SfmlSimulationVisualizer::drawGraph(sf::RenderTarget &target) {
  if (graphCacheDirty_) {
    rebuildGraphCache();
  }

  const auto old = target.getView();
  target.setView(sceneView_);

  if (edgesVA_) {
    target.draw(*edgesVA_);
  }

  if (!nodesVA_.empty()) {
    const float rW = std::max(0.f, Parameters::nodeRadius());

    sf::CircleShape node(rW);
    node.setOrigin(rW, rW);

    for (const sf::Vector2f &wpos : nodesVA_) {
      node.setPosition(wpos);
      target.draw(node);
    }
  }

  target.setView(old);
}
