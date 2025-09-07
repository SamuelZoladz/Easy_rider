/**
 * @file CongestionModel.h
 * @brief Tiered (halving) congestion model driven by per-edge Road capacity.
 *
 * Rules (per directed edge):
 *  - Let x = Road capacity (max vehicles "comfortably" on edge).
 *  - If N <= x         -> v_eff = v_free.
 *  - If x < N <= 2x    -> v_eff = v_free / 2.
 *  - If 2x < N <= 3x   -> v_eff = v_free / 4.
 *  - If 3x < N <= 4x   -> v_eff = v_free / 8.
 *  - In general: v_eff = v_free / 2^(ceil(N/x) - 1).
 */

#ifndef CONGESTION_MODEL_H
#define CONGESTION_MODEL_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>

class Road;

/**
 * @brief Key of a directed edge: fromId -> toId.
 */
using EdgeKey = std::pair<int, int>;

/**
 * @brief Hash for EdgeKey (pair<int,int>).
 */
struct EdgeKeyHash {
  std::size_t operator()(const EdgeKey &k) const noexcept {
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(k.first))
            << 32) ^
           static_cast<std::uint32_t>(k.second);
  }
};

/**
 * @brief Runtime state of a single directed edge.
 *
 * vehicles: live count on the edge.
 * speedLimitOverride: temporary limit applied to v_free (e.g., incident,
 * works). If set, it caps the free-flow speed before applying halving tiers.
 */
struct EdgeState {
  int vehicles{0};
  std::optional<double> speedLimitOverride{};
};

/**
 * @class CongestionModel
 * @brief Discrete tier-based halving model using capacity from Road.
 *
 * Required Road API:
 *   int    Road::getCapacityVehicles() const;  // returns x
 *   int    Road::getFromId() const;
 *   int    Road::getToId() const;
 *   int    Road::getMaxSpeed() const;          // v_free in model units
 *   double Road::getLength() const;            // used for time computation
 */
class CongestionModel {
public:
  CongestionModel() = default;
  ~CongestionModel() = default;
  CongestionModel(const CongestionModel &) = default;
  CongestionModel &operator=(const CongestionModel &) = default;
  CongestionModel(CongestionModel &&) noexcept = default;
  CongestionModel &operator=(CongestionModel &&) noexcept = default;

  /// @brief Utility to build an EdgeKey.
  static constexpr EdgeKey makeEdgeKey(int fromId, int toId) noexcept {
    return {fromId, toId};
  }

  /// @brief Call when a vehicle enters a directed edge (fromId, toId).
  void onEnterEdge(const EdgeKey &edge);

  /// @brief Call when a vehicle exits a directed edge (fromId, toId).
  void onExitEdge(const EdgeKey &edge);

  /**
   * @brief Set default capacity if Road reports non-positive capacity.
   * @param cap Default "x" (vehicles). Must be >= 1.
   */
  void setDefaultCapacityVehicles(int cap) {
    defaultCapacityVehicles_ = std::max(1, cap);
  }

  /**
   * @brief Get effective speed on edge according to the halving rule.
   * @return Effective speed in the same units as Road::getMaxSpeed().
   */
  [[nodiscard]] double effectiveSpeed(const Road &road) const;

  /**
   * @brief Get travel time over road for a vehicle with its own max speed cap.
   * @param road               Road edge.
   * @param vehicleMaxSpeed    Vehicle's own max speed (cap).
   * @return Time = length / min(vehicleMaxSpeed, effectiveSpeed(road))
   */
  [[nodiscard]] double edgeTime(const Road &road, int vehicleMaxSpeed) const;

private:
  /// @brief Resolve capacity x for a given road (falls back to default if <=
  /// 0).
  [[nodiscard]] int capacityFor(const Road &road) const;

  // Live per-edge state (counts and temporary limits).
  std::unordered_map<EdgeKey, EdgeState, EdgeKeyHash> state_;

  // Fallback capacity (vehicles) used when Road::getCapacityVehicles() <= 0.
  int defaultCapacityVehicles_{10};
};

#endif // CONGESTION_MODEL_H
