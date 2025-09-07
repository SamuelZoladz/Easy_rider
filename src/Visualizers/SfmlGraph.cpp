#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>

#include "Easy_rider/Parameters/Parameters.h"

void SfmlSimulationVisualizer::rebuildGraphCache() {
  const auto data = makeGraphDrawData(simulation_->graph());

  // Base thickness
  const float baseT = std::max(0.1f, Parameters::baseEdgeThickness());

  edgesVA_ = std::make_unique<sf::VertexArray>(sf::Triangles);

  // Gathering min/max speeds to normalize thickness
  const auto &modelEdges = simulation_->graph().getEdges();
  float minSpeed = std::numeric_limits<float>::infinity();
  float maxSpeed = -std::numeric_limits<float>::infinity();
  for (const auto &e : modelEdges) {
    const float v = static_cast<float>(e.getMaxSpeed());
    if (v < minSpeed)
      minSpeed = v;
    if (v > maxSpeed)
      maxSpeed = v;
  }
  const float denom = std::max(1e-6f, maxSpeed - minSpeed);

  for (std::size_t i = 0; i < data.edges.size(); ++i) {
    const auto &edgeIdxPair = data.edges[i];
    const sf::Vector2f a = data.nodePositions.at(edgeIdxPair.first);
    const sf::Vector2f b = data.nodePositions.at(edgeIdxPair.second);

    // speed for this edge
    const float v = static_cast<float>(modelEdges[i].getMaxSpeed());
    const float tNorm = (denom > 1e-6f) ? (v - minSpeed) / denom : 0.5f;
    const float thickness =
        baseT * (Parameters::slowEdgeThicknessFactor() +
                 tNorm * (Parameters::fastEdgeThicknessFactor() -
                          Parameters::slowEdgeThicknessFactor()));
    const float halfTWorld = thickness * 0.5f;

    sf::Vector2f d(b.x - a.x, b.y - a.y);
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 1e-4f)
      continue;

    const sf::Vector2f n(-d.y / len * halfTWorld, d.x / len * halfTWorld);

    const sf::Vector2f v0(a.x + n.x, a.y + n.y);
    const sf::Vector2f v1(b.x + n.x, b.y + n.y);
    const sf::Vector2f v2(b.x - n.x, b.y - n.y);
    const sf::Vector2f v3(a.x - n.x, a.y - n.y);

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
