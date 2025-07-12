/**
 * @file Node.h
 * @brief Declaration of the Node class representing a point in 2D space.
 */

#ifndef NODE_H
#define NODE_H

#include <utility>

/**
 * @class Node
 * @brief Represents a point (node) in a 2D coordinate system.
 */
class Node {
public:
  /**
   * @brief Default constructor.
   */
  Node();

  /**
   * @brief Parameterized constructor.
   * @param x The x-coordinate.
   * @param y The y-coordinate.
   */
  Node(int x, int y);

  /**
   * @brief Sets a new position for the node.
   * @param x The new x-coordinate.
   * @param y The new y-coordinate.
   */
  void setPosition(int x, int y);

  /**
   * @brief Retrieves the current position of the node.
   * @return A std::pair where first is x-coordinate and second is y-coordinate.
   */
  std::pair<int, int> getPosition() const;

  /**
   * @brief Retrieves the x-coordinate.
   * @return The current x-coordinate.
   */
  int getX() const;

  /**
   * @brief Retrieves the y-coordinate.
   * @return The current y-coordinate.
   */
  int getY() const;

private:
  int x_; /**< The x-coordinate. */
  int y_; /**< The y-coordinate. */
};

#endif // NODE_H
