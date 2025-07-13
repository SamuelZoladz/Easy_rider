/**
 * @file Intersection.cpp
 * @brief Definitions for the Intersection class methods.
 */

#include "Easy_rider/Intersection.h"

Intersection::Intersection() : x_(0), y_(0) {}

Intersection::Intersection(int x, int y) : x_(x), y_(y) {}

void Intersection::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

std::pair<int, int> Intersection::getPosition() const { return {x_, y_}; }

int Intersection::getX() const { return x_; }

int Intersection::getY() const { return y_; }
