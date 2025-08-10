/**
 * @file Road.h
 * @brief Declaration of the Road class representing a connection between two
 * nodes (intersections) by identifiers.
 */
#ifndef ROAD_H
#define ROAD_H

#include "Intersection.h"
#include <utility>

/**
 * @class Road
 * @brief Represents a direcred road connecting two intersections (by ids), with
 * a computed length and a maximum speed.
 */
class Road {
public:
  /**
   * @brief Default constructor is deleted.
   */
  Road() = delete;

  /**
   * @brief Constructs a road from two intersections and computes its length.
   * @param from Source intersection (its id and position are used).
   * @param to   Target intersection (its id and position are used).
   * @param maxSpeed Maximum allowed speed on this road.
   */
  Road(const Intersection &from, const Intersection &to, int maxSpeed);

  /// @return Source node id.
  int getFromId() const;

  /// @return Target node id.
  int getToId() const;

  /// @return Euclidean length.
  double getLength() const;

  /// @return Maximum allowed speed.
  int getMaxSpeed() const;

private:
  int fromId_;    /**< Source node id. */
  int toId_;      /**< Target node id. */
  double length_; /**< Cached Euclidean distance between endpoints. */
  int maxSpeed_;  /**< Maximum allowed speed along this road. */

  /// @brief Helper to compute distance from two positions.
  static double computeLength(const std::pair<int, int> &a,
                              const std::pair<int, int> &b);
};

#endif // ROAD_H
