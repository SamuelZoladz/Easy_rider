/**
 * @file Road.h
 * @brief Declaration of the Road class representing a connection between two
 * Intersections.
 */
#ifndef ROAD_H
#define ROAD_H

#include "Intersection.h"

/**
 * @class Road
 * @brief Represents a road connecting two Intersections in 2D space, with a
 * computed length and a maximum speed.
 */
class Road {
public:
  /**
   * @brief Default constructor.
   *        Initializes both endpoints at (0,0), length to 0.0, and maxSpeed to
   * 0.
   */
  Road();

  /**
   * @brief Parameterized constructor.
   * @param from The starting Intersection.
   * @param to The ending Intersection.
   * @param maxSpeed The maximum allowed speed on this road.
   *
   * Automatically computes the Euclidean length between \p from and \p to.
   */
  Road(const Intersection &from, const Intersection &to, int maxSpeed);

  /**
   * @brief Retrieves the starting Intersection.
   * @return A copy of the 'from' Intersection.
   */
  Intersection getFrom() const;

  /**
   * @brief Retrieves the ending Intersection.
   * @return A copy of the 'to' Intersection.
   */
  Intersection getTo() const;

  /**
   * @brief Retrieves the computed length of the road.
   * @return The Euclidean distance between 'from' and 'to'.
   */
  double getLength() const;

  /**
   * @brief Retrieves the maximum speed for this road.
   * @return The maximum speed (int).
   */
  int getMaxSpeed() const;

private:
  Intersection from_; /**< The starting intersection. */
  Intersection to_;   /**< The ending intersection. */
  double length_;     /**< Computed Euclidean distance between from_ and to_. */
  int maxSpeed_;      /**< Maximum speed allowed along this road. */

  /**
   * @brief Computes and updates length_ as the distance between from_ and to_.
   */
  void computeLength();
};

#endif // ROAD_H
