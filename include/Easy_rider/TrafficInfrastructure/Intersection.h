/**
 * @file Intersection.h
 * @brief Declaration of the Intersection class representing a point in 2D space
 * with auto-assigned ids.
 */

#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <utility>

/**
 * @class Intersection
 * @brief Represents a point (intersection) in a 2D coordinate system. IDs are
 * auto-assigned from a static counter.
 */
class Intersection {
public:
  /**
   * @brief Default-constructs an intersection at (0, 0) with an auto-assigned
   * id.
   */
  Intersection();

  /**
   * @brief Constructs an intersection at the given coordinates with an
   * auto-assigned id.
   * @param x X coordinate.
   * @param y Y coordinate.
   */
  Intersection(int x, int y);

  /**
   * @brief Sets the (x, y) position.
   * @param x New x-coordinate.
   * @param y New y-coordinate.
   */
  void setPosition(int x, int y);

  /**
   * @brief Returns the current (x, y) position.
   * @return A pair {x, y}.
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

  /**
   * @brief Retrieves the unique identifier of this intersection.
   */
  int getId() const;

private:
  int id_; /**< Unique identifier of the intersection (node id). */
  int x_;  /**< The x-coordinate. */
  int y_;  /**< The y-coordinate. */

  static int
      s_nextId_; /**< Global auto-increment id source (single-threaded). */
};

#endif // INTERSECTION_H
