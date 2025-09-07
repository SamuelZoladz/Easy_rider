/**
 * @file VisualizerUtils.h
 * @brief Backend-agnostic helpers to turn simulation data into drawable forms.
 */
#ifndef VISUALIZER_UTILS_H
#define VISUALIZER_UTILS_H

#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"

#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <utility>
#include <vector>

class Simulation;

/**
 * @brief Pre-baked graph geometry ready for drawing.
 *
 * - nodePositions[i] is the world position for node index @p i.
 * - edges contain pairs of node indices (u, v) drawn as straight segments.
 *   The pair values are indices into nodePositions.
 *
 * No guarantee is made about edge direction or (u <= v) ordering.
 */
struct GraphDrawData {
  std::vector<sf::Vector2f> nodePositions; ///< World-space node centers.
  std::vector<std::pair<std::size_t, std::size_t>> edges; ///< Node index pairs.
};

/**
 * @brief Extract drawable graph data from the traffic infrastructure graph.
 *
 * The resulting GraphDrawData:
 *  - enumerates intersections as nodePositions (using their world locations),
 *  - lists roads as index pairs in edges (by their endpoint intersections).
 */
[[nodiscard]] GraphDrawData
makeGraphDrawData(const Graph<Intersection, Road> &g);

/**
 * @brief Extract current vehicle world positions from a Simulation.
 *
 * The order of returned positions is unspecified and may change frame-to-frame.
 */
[[nodiscard]] std::vector<sf::Vector2f>
extractVehiclePositions(const Simulation &sim);

#endif // VISUALIZER_UTILS_H
