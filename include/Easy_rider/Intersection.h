/**
 * @file Intersection.h
 * @brief Declaration of the Intersection class representing a point in 2D
 * space.
 */

#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <utility>

/**
 * @class Intersection
 * @brief Represents a point (intersection) in a 2D coordinate system.
 */
class Intersection {
public:
  /**
   * @brief Default constructor.
   */
  Intersection();

  /**
   * @brief Parameterized constructor.
   * @param x The x-coordinate.
   * @param y The y-coordinate.
   */
  Intersection(int x, int y);

  /**
   * @brief Sets a new position for the intersection.
   * @param x The new x-coordinate.
   * @param y The new y-coordinate.
   */
  void setPosition(int x, int y);

  /**
   * @brief Retrieves the current position of the intersection.
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

#endif // INTERSECTION_H
