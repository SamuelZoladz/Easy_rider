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
   */
  StreetGenerator(size_t k, int defaultSpeed);

  void generate(Graph<Intersection, Road> &graph) override;

private:
  size_t k_;
  int defaultSpeed_;
};

#endif // STREET_GENERATOR_H
