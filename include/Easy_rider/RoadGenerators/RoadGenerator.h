#ifndef ROAD_GENERATOR_H
#define ROAD_GENERATOR_H

#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"
/**
 * @brief Abstract base class for generating roads on a
 * Graph<Intersection,Road>.
 */
class RoadGenerator {
public:
  virtual ~RoadGenerator() = default;

  /**
   * @brief Append new bidirectional roads into the graph (no duplicates).
   */
  virtual void generate(Graph<Intersection, Road> &graph) = 0;

protected:
  /**
   * @brief Compute the Euclidean distance between two intersections.
   * @param a First intersection.
   * @param b Second intersection.
   * @return The Euclidean distance between a and b.
   */
  static double euclid(const Intersection &a, const Intersection &b);
};

#endif // ROAD_GENERATOR_H
