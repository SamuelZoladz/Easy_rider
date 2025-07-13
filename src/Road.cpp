/**
 * @file Road.cpp
 * @brief Definitions for the Road class methods.
 */

#include "Easy_rider/Road.h"
#include <cmath>

Road::Road()
    : from_(Intersection()), to_(Intersection()), length_(0.0), maxSpeed_(0) {
  // length_ is already zero
}

Road::Road(const Intersection &from, const Intersection &to, int maxSpeed)
    : from_(from), to_(to), maxSpeed_(maxSpeed) {
  computeLength();
}

void Road::computeLength() {
  const int dx = to_.getX() - from_.getX();
  const int dy = to_.getY() - from_.getY();
  length_ = std::sqrt(static_cast<double>(dx * dx + dy * dy));
}

Intersection Road::getFrom() const { return from_; }

Intersection Road::getTo() const { return to_; }

double Road::getLength() const { return length_; }

int Road::getMaxSpeed() const { return maxSpeed_; }
