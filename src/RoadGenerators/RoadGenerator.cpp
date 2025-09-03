#include "Easy_rider/RoadGenerators/RoadGenerator.h"

#include <cmath>

double RoadGenerator::euclid(const Intersection &a, const Intersection &b) {
  auto [x1, y1] = a.getPosition();
  auto [x2, y2] = b.getPosition();
  return std::hypot(static_cast<double>(x1 - x2), static_cast<double>(y1 - y2));
}
