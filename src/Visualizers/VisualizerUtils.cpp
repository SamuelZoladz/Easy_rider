#include "Easy_rider/Visualizers/VisualizerUtils.h"

#include "Easy_rider/Simulation/Simulation.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace viz {

static Vec2 nodePosAdapter(const Intersection &n) {
  auto [x, y] = n.getPosition();
  return Vec2{static_cast<float>(x), static_cast<float>(y)};
}

static inline std::pair<std::size_t, std::size_t>
edgeNodesAdapter(const Graph<Intersection, Road> &g) = delete;

static inline std::pair<std::size_t, std::size_t>
edgeNodesAdapter(const Graph<Intersection, Road> &g, std::size_t edgeIdx) {
  const auto &e = g.getEdges().at(edgeIdx);
  const int fromId = e.getFromId();
  const int toId = e.getToId();
  const std::size_t ia = g.indexOfId(fromId);
  const std::size_t ib = g.indexOfId(toId);
  return {ia, ib};
}

GraphDrawData makeGraphDrawData(const Graph<Intersection, Road> &g) {
  GraphDrawData out;

  const auto &nodes = g.getNodes();
  out.nodePositions.reserve(nodes.size());
  for (const auto &n : nodes) {
    out.nodePositions.push_back(nodePosAdapter(n));
  }

  const auto &edges = g.getEdges();
  out.edges.reserve(edges.size());
  for (std::size_t i = 0; i < edges.size(); ++i) {
    out.edges.push_back(edgeNodesAdapter(g, i));
  }
  return out;
}

std::vector<Vec2> extractVehiclePositions(const Simulation &sim) {
  std::vector<Vec2> pts;
  const auto &G = sim.graph();
  const auto snap = sim.snapshot();
  pts.reserve(snap.size());

  for (const auto &v : snap) {
    auto [x1, y1] = G.positionOf(v.fromId);
    auto [x2, y2] = G.positionOf(v.toId);

    const float dx = static_cast<float>(x2 - x1);
    const float dy = static_cast<float>(y2 - y1);
    const double len = std::hypot(dx, dy);

    float t = 0.0f;
    if (len > 1e-6) {
      t = static_cast<float>(std::clamp(v.sOnEdge / len, 0.0, 1.0));
    }

    pts.push_back(
        Vec2{static_cast<float>(x1) + dx * t, static_cast<float>(y1) + dy * t});
  }
  return pts;
}

} // namespace viz
