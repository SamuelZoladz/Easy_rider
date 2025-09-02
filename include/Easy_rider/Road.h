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
 * @brief Represents a directed road connecting two intersections (by ids), with
 * a computed length, a maximum speed, and a capacity (vehicles concurrently on
 * edge).
 */
class Road {
public:
  /**
   * @brief Default constructor is deleted.
   */
  Road() = delete;

  /**
   * @brief Constructs a road from two intersections and computes its length.
   * @param from      Source intersection (its id and position are used).
   * @param to        Target intersection (its id and position are used).
   * @param maxSpeed  Maximum allowed speed on this road.
   * @note Capacity defaults to a constant (10 vehicles).
   */
  Road(const Intersection &from, const Intersection &to, int maxSpeed);

  /**
   * @brief Constructs a road with explicit capacity (vehicles).
   * @param from               Source intersection.
   * @param to                 Target intersection.
   * @param maxSpeed           Maximum allowed speed on this road.
   * @param capacityVehicles   Capacity "x" (current vehicles allowed before
   * halving tiers).
   */
  Road(const Intersection &from, const Intersection &to, int maxSpeed,
       int capacityVehicles);

  /// @return Source node id.
  int getFromId() const;

  /// @return Target node id.
  int getToId() const;

  /// @return Euclidean length.
  double getLength() const;

  /// @return Maximum allowed speed.
  int getMaxSpeed() const;

  /// @return Capacity (vehicles) reported by this road.
  int getCapacityVehicles() const;

private:
  int fromId_;           /**< Source node id. */
  int toId_;             /**< Target node id. */
  double length_;        /**< Cached Euclidean distance between endpoints. */
  int maxSpeed_;         /**< Maximum allowed speed along this road. */
  int capacityVehicles_; /**< Capacity "x" (vehicles concurrently on edge). */

  inline static constexpr int kDefaultCapacityVehicles = 10;
  /// @brief Helper to compute distance from two positions.
  static double computeLength(const std::pair<int, int> &a,
                              const std::pair<int, int> &b);
};

#endif // ROAD_H
