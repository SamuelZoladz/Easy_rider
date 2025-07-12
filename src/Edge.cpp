/**
 * @file Edge.cpp
 * @brief Definitions for the Edge class methods.
 */

#include "Easy_rider/Edge.h"
#include <cmath>

Edge::Edge() : from_(Node()), to_(Node()), length_(0.0), maxSpeed_(0) {
  // length_ is already zero
}

Edge::Edge(const Node &from, const Node &to, int maxSpeed)
    : from_(from), to_(to), maxSpeed_(maxSpeed) {
  computeLength();
}

void Edge::computeLength() {
  const int dx = to_.getX() - from_.getX();
  const int dy = to_.getY() - from_.getY();
  length_ = std::sqrt(static_cast<double>(dx * dx + dy * dy));
}

Node Edge::getFrom() const { return from_; }

Node Edge::getTo() const { return to_; }

double Edge::getLength() const { return length_; }

int Edge::getMaxSpeed() const { return maxSpeed_; }
