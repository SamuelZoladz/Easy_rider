#ifndef MOTORWAY_GENERATOR_H
#define MOTORWAY_GENERATOR_H

#include "RoadGenerator.h"

/**
 * @brief Builds a continuous "motorway" route by:
 *   1. Finding the two farthest intersections A and B.
 *   2. Computing a dynamic perpendicular threshold = thresholdRatio *
 * distance(A,B).
 *   3. Selecting all intersections within that threshold of the straight line
 * A->B.
 *   4. Sorting them by their projection along A->B.
 *   5. Connecting them in sequence (A->…->B) with bidirectional edges.
 */
class MotorwayGenerator : public RoadGenerator {
public:
  /**
   * @param thresholdRatio  Fraction of AB's length used as max perpendicular
   *                        distance for including nodes (e.g. 0.1 = 10%).
   * @param defaultSpeed    Speed to assign to each motorway segment.
   * @param capacity Capacity (vehicles) for every new Road.
   */
  MotorwayGenerator(double thresholdRatio, int defaultSpeed, int capacity);

  /**
   * @brief Append new bidirectional motorways into the graph (no duplicates).
   */
  void generate(Graph<Intersection, Road> &graph) override;

private:
  double thresholdRatio_;
  int defaultSpeed_;
  int capacity_;
};

#endif // MOTORWAY_GENERATOR_H
