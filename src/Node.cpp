/**
 * @file Node.cpp
 * @brief Definitions for the Node class methods.
 */

#include "Easy_rider/Node.h"

Node::Node() : x_(0), y_(0) {}

Node::Node(int x, int y) : x_(x), y_(y) {}

void Node::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

std::pair<int, int> Node::getPosition() const { return {x_, y_}; }

int Node::getX() const { return x_; }

int Node::getY() const { return y_; }
