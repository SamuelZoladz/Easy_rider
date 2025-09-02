#ifndef HIGHWAY_GENERATOR_H
#define HIGHWAY_GENERATOR_H

#include "RoadGenerator.h"

/**
 * @brief Builds a minimum spanning tree (Kruskal) over all intersections —
 *        Highway generator.
 *
 * @note This implementation deviates from the standard Kruskal algorithm by
 *       enforcing a planar constraint: edges are only added when *both*
 *       directed variants (A->B and B->A) can be inserted without crossing
 *       existing roads. As a result, it does *not* always produce the exact
 *       minimum spanning tree as returned by a pure Kruskal run.
 */
class HighwayGenerator : public RoadGenerator {
public:
  /**
   * @param defaultSpeed    Speed for every new Road.
   * @param capacity Capacity (vehicles) for every new Road.
   */
  explicit HighwayGenerator(int defaultSpeed, int capacity);

  /**
   * @brief Append new bidirectional highways into the graph (no duplicates).
   */
  void generate(Graph<Intersection, Road> &graph) override;

private:
  int defaultSpeed_;
  int capacity_;
};

#endif // HIGHWAY_GENERATOR_H
