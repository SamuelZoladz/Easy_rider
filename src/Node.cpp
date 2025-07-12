/**
 * @file Node.cpp
 * @brief Definitions for the Node class methods.
 */

#include "Easy_rider/Node.h"

/**
 * @brief Default constructor.
 *
 * Initializes the node at the origin (0,0).
 */
Node::Node() : x_(0), y_(0) {}

/**
 * @brief Parameterized constructor.
 * @param x The x-coordinate to initialize.
 * @param y The y-coordinate to initialize.
 */
Node::Node(int x, int y) : x_(x), y_(y) {}

/**
 * @brief Sets a new position.
 * @param x The new x-coordinate.
 * @param y The new y-coordinate.
 */
void Node::setPosition(int x, int y) {
  x_ = x;
  y_ = y;
}

/**
 * @brief Gets the current position.
 * @return A std::pair containing (x, y).
 */
std::pair<int, int> Node::getPosition() const { return {x_, y_}; }

/**
 * @brief Gets the x-coordinate.
 * @return The x-coordinate.
 */
int Node::getX() const { return x_; }

/**
 * @brief Gets the y-coordinate.
 * @return The y-coordinate.
 */
int Node::getY() const { return y_; }
