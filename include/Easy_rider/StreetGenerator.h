#ifndef STREET_GENERATOR_H
#define STREET_GENERATOR_H

#include "RoadGenerator.h"

/**
 * @brief For each intersection, connects it to its k nearest neighbors -
 * Street generator.
 */
class StreetGenerator : public RoadGenerator {
public:
  /**
   * @param k            Number of neighbors.
   * @param defaultSpeed Speed for every new Road.
   * @param capacity Capacity (vehicles) for every new Road.
   */
  StreetGenerator(size_t k, int defaultSpeed, int capacity);

  void generate(Graph<Intersection, Road> &graph) override;

private:
  size_t k_;
  int defaultSpeed_;
  int capacity_;
};

#endif // STREET_GENERATOR_H
