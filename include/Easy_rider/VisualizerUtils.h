#ifndef VISUALIZER_UTILS_H
#define VISUALIZER_UTILS_H

#include "Graph.h"
#include "Intersection.h"
#include "Road.h"
#include <cstddef>
#include <utility>
#include <vector>

#include "Simulation.h"
/**
 * @file VisualizerUtils.h
 * @brief Backend-agnostic helpers for turning the graph/sim into drawable data.
 *
 * Keep any non-trivial extraction or formatting logic here, so rendering
 * classes remain focused purely on drawing.
 */

namespace viz {

/**
 * @brief Lightweight 2D vector detached from any graphics library.
 */
struct Vec2 {
  float x{0.0f};
  float y{0.0f};
};

/**
 * @brief Pre-baked graph geometry ready to be drawn.
 *
 * - nodePositions[i] is the world position for node index i
 * - edges store pairs of node indices (u, v) to be drawn as segments
 */
struct GraphDrawData {
  std::vector<Vec2> nodePositions;
  std::vector<std::pair<std::size_t, std::size_t>> edges;
};

/**
 * @brief Compute axis-aligned world bounds of a set of points.
 * @return {minX, minY, maxX, maxY}
 */
inline std::tuple<float, float, float, float>
bounds(const std::vector<Vec2> &pts) {
  if (pts.empty())
    return {0, 0, 0, 0};
  float minX = pts[0].x, maxX = pts[0].x;
  float minY = pts[0].y, maxY = pts[0].y;
  for (const auto &p : pts) {
    if (p.x < minX)
      minX = p.x;
    if (p.x > maxX)
      maxX = p.x;
    if (p.y < minY)
      minY = p.y;
    if (p.y > maxY)
      maxY = p.y;
  }
  return {minX, minY, maxX, maxY};
}

/**
 * @brief Extract drawable graph data.
 */
GraphDrawData makeGraphDrawData(const Graph<Intersection, Road> &g);

/**
 * @brief Extract current vehicle world positions from Simulation.
 */
std::vector<Vec2> extractVehiclePositions(const Simulation &sim);

} // namespace viz

#endif // VISUALIZER_UTILS_H
