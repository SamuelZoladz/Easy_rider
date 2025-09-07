#include "Easy_rider/Visualizers/VisualizerUtils.h"

#include "Easy_rider/Simulation/Simulation.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace {
// Convert an Intersection's (double,double) position to sf::Vector2f.
sf::Vector2f nodePos(const Intersection &n) {
  auto [x, y] = n.getPosition();
  return {static_cast<float>(x), static_cast<float>(y)};
}

// Return indices (in the graph's node array) of an edge's endpoints.
std::pair<std::size_t, std::size_t>
edgeNodeIndices(const Graph<Intersection, Road> &g, std::size_t edgeIdx) {
  const auto &e = g.getEdges().at(edgeIdx);
  const std::size_t ia = g.indexOfId(e.getFromId());
  const std::size_t ib = g.indexOfId(e.getToId());
  return {ia, ib};
}
} // namespace

// Build a lightweight draw cache: world positions for nodes and
// (nodeIndexA, nodeIndexB) pairs for edges.
GraphDrawData makeGraphDrawData(const Graph<Intersection, Road> &g) {
  GraphDrawData out;

  const auto &nodes = g.getNodes();
  out.nodePositions.reserve(nodes.size());
  for (const auto &n : nodes) {
    out.nodePositions.push_back(nodePos(n));
  }

  const auto &edges = g.getEdges();
  out.edges.reserve(edges.size());
  for (std::size_t i = 0; i < edges.size(); ++i) {
    out.edges.push_back(edgeNodeIndices(g, i));
  }

  return out;
}

// Extract current vehicle positions in world coordinates by linearly
// interpolating along each vehicle's edge based on traveled distance.
std::vector<sf::Vector2f> extractVehiclePositions(const Simulation &sim) {
  std::vector<sf::Vector2f> pts;

  const auto &graph = sim.graph();
  const auto snapshot = sim.snapshot();
  pts.reserve(snapshot.size());

  for (const auto &v : snapshot) {
    // Endpoints of the edge the vehicle is currently on.
    auto [x1, y1] = graph.positionOf(v.fromId);
    auto [x2, y2] = graph.positionOf(v.toId);

    const float dx = static_cast<float>(x2 - x1);
    const float dy = static_cast<float>(y2 - y1);
    const double len = std::hypot(dx, dy);

    float t = 0.0f;
    if (len > 1e-6) {
      t = static_cast<float>(std::clamp(v.sOnEdge / len, 0.0, 1.0));
    }

    pts.emplace_back(static_cast<float>(x1) + dx * t,
                     static_cast<float>(y1) + dy * t);
  }

  return pts;
}
