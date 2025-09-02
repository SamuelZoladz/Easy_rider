/**
 * @file CongestionModel.h
 * @brief Tiered (halving) congestion model driven by Road capacity.
 *
 * Rules (per edge):
 * - Let x = Road capacity (max vehicles "comfortably" on edge).
 * - If N <= x  -> v_eff = v_free.
 * - If x < N <= 2x  -> v_eff = v_free / 2.
 * - If 2x < N <= 3x -> v_eff = v_free / 4.
 * - If 3x < N <= 4x -> v_eff = v_free / 8.
 * - And so on: v_eff = v_free / 2^(ceil(N/x) - 1).
 */
#ifndef CONGESTION_MODEL_H
#define CONGESTION_MODEL_H

#include "Road.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>

/**
 * @brief Key of a directed edge: fromId -> toId.
 */
using EdgeKey = std::pair<int, int>;

struct EdgeKeyHash {
  std::size_t operator()(const EdgeKey &k) const noexcept {
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(k.first))
            << 32) ^
           static_cast<std::uint32_t>(k.second);
  }
};

/**
 * @brief Runtime state of a single directed edge.
 */
struct EdgeState {
  int vehicles{0}; /**< Vehicles currently on the edge. */
  std::optional<double>
      speedLimitOverride{}; /**< Temporary limit (e.g., incident). */
};

/**
 * @class CongestionModel
 * @brief Discrete tier-based halving model using capacity from Road.
 *
 * Required Road API:
 *   int Road::getCapacityVehicles() const;  // returns x
 *   int Road::getFromId() const;
 *   int Road::getToId() const;
 *   int Road::getMaxSpeed() const;          // v_free in model units
 *   double Road::getLength() const;         // used only for time computation
 */
class CongestionModel {
public:
  CongestionModel() = default;

  /// @brief Call when a vehicle enters a directed edge (fromId, toId).
  void onEnterEdge(const std::pair<int, int> &edge);

  /// @brief Call when a vehicle exits a directed edge (fromId, toId).
  void onExitEdge(const std::pair<int, int> &edge);

  /**
   * @brief Set default capacity if Road reports non-positive capacity.
   * @param cap Default "x" (vehicles). Must be >= 1.
   */
  void setDefaultCapacityVehicles(int cap) {
    defaultCapacityVehicles_ = std::max(1, cap);
  }

  /**
   * @brief Get effective speed on edge e according to tiered halving rule.
   * @return Effective speed in the same units as Road::getMaxSpeed().
   */
  double effectiveSpeed(const Road &e) const;

  /**
   * @brief Get travel time over e for a vehicle with max speed limit.
   * @param e                 Road edge.
   * @param vehicleMaxSpeed   Vehicle's own max speed (cap).
   * @return Time = length / min(vehicleMaxSpeed, effectiveSpeed(e))
   */
  double edgeTime(const Road &e, int vehicleMaxSpeed) const;

private:
  int capacityFor(const Road &e) const;

  std::unordered_map<EdgeKey, EdgeState, EdgeKeyHash> state_;
  int defaultCapacityVehicles_{10}; // fallback x when Road capacity <= 0
};

#endif // CONGESTION_MODEL_H
