/**
 * @file Road.cpp
 * @brief Definitions for the Road class methods.
 */
#include "Easy_rider/Road.h"

#include <cassert>
#include <cmath>
#include <utility>

Road::Road(const Intersection &from, const Intersection &to, int maxSpeed)
    : fromId_(from.getId()), toId_(to.getId()),
      length_(computeLength(from.getPosition(), to.getPosition())),
      maxSpeed_(maxSpeed) {
  assert(from.getId() != to.getId() && "Self-loop roads are not allowed");
}

int Road::getFromId() const { return fromId_; }
int Road::getToId() const { return toId_; }
double Road::getLength() const { return length_; }
int Road::getMaxSpeed() const { return maxSpeed_; }

double Road::computeLength(const std::pair<int, int> &a,
                           const std::pair<int, int> &b) {
  const double dx = static_cast<double>(b.first) - static_cast<double>(a.first);
  const double dy =
      static_cast<double>(b.second) - static_cast<double>(a.second);
  return std::hypot(dx, dy);
}
