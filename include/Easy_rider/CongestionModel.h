#ifndef CONGESTION_MODEL_H
#define CONGESTION_MODEL_H

#include "Road.h"
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

struct EdgeState {
  int vehicles{0};
  std::optional<double> speedLimitOverride{}; // np. zdarzenia, blokady
};

/**
 * @class CongestionModel
 * @brief Tracks per-edge load and provides effective speeds/time for routing.
 *
 * Effective speed model:
 *  v_eff = min(road.maxSpeed, speedLimit(edge)) * f(load)
 * where f(load) is a piecewise factor in (0, 1].
 *
 * Additionally, a very low speedLimit can emulate incidents or blocks.
 */
class CongestionModel {
public:
  CongestionModel() = default;

  /// @name Load accounting
  ///@{
  void onEnterEdge(const std::pair<int, int> &edge);
  void onExitEdge(const std::pair<int, int> &edge);
  ///@}

  int vehiclesOnEdge(const EdgeKey &k) const;

  /// @name Event / incident API (optional)
  ///@{
  void setEdgeSpeedLimit(const EdgeKey &k, double limit);
  void clearEdgeSpeedLimit(const EdgeKey &k);
  ///@}

  double effectiveSpeed(const Road &e) const; // [jedn. prędkości]
  double edgeTime(const Road &e, int vehicleMaxSpeed) const; // [czas]

  /// @brief Configure thresholds.
  void setCongestionThreshold(int thr) { threshold_ = thr; }

  static void setActive(CongestionModel *) {}

private:
  std::unordered_map<EdgeKey, EdgeState, EdgeKeyHash> state_;
  int threshold_{5};
};

#endif // CONGESTION_MODEL_H
