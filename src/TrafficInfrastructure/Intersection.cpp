/**
 * @file Intersection.cpp
 * @brief Definitions for the Intersection class methods (single-threaded auto
 * id).
 */

#include "Easy_rider/TrafficInfrastructure/Intersection.h"

int Intersection::s_nextId_ = 0;

Intersection::Intersection() : id_(s_nextId_++), x_(0), y_(0) {}

Intersection::Intersection(int x, int y) : id_(s_nextId_++), x_(x), y_(y) {}

void Intersection::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

std::pair<int, int> Intersection::getPosition() const { return {x_, y_}; }

int Intersection::getX() const { return x_; }

int Intersection::getY() const { return y_; }

int Intersection::getId() const { return id_; }
