#ifndef CONGESTION_MODEL_H
#define CONGESTION_MODEL_H

#include "Graph.h"
#include "Intersection.h"
#include "Road.h"
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>

/**
 * @brief Key of a directed edge: fromId -> toId.
 */
struct EdgeKey {
  int fromId{};
  int toId{};
  bool operator==(const EdgeKey &o) const {
    return fromId == o.fromId && toId == o.toId;
  }
};

struct EdgeKeyHash {
  std::size_t operator()(const EdgeKey &k) const noexcept {
    return (static_cast<std::size_t>(k.fromId) << 32) ^
           static_cast<std::size_t>(k.toId);
  }
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
  int vehiclesOnEdge(const EdgeKey &k) const;
  ///@}

  /// @brief Returns true if the edge is currently congested (load >=
  /// threshold).
  bool isCongested(const Road &e) const;

  /// @brief Compute effective speed for a road (always > 0).
  double effectiveSpeed(const Road &e) const;

  /// @brief Compute travel time for a road (length / v_eff).
  double edgeTime(const Road &e) const;

  /// @brief Global function pointer adapter for routing strategies.
  static double StaticEdgeTime(const Road &e);

  /// @brief Install this model as the active provider for StaticEdgeTime.
  static void setActive(CongestionModel *m) { s_active_ = m; }

  /// @name Event / incident API (optional)
  ///@{
  void setEdgeSpeedLimit(const EdgeKey &k, double limit);
  void clearEdgeSpeedLimit(const EdgeKey &k);
  ///@}

  /// @brief Configure thresholds.
  void setCongestionThreshold(int thr) { threshold_ = thr; }

private:
  static CongestionModel *s_active_;

  std::unordered_map<EdgeKey, int, EdgeKeyHash> load_;
  std::unordered_map<EdgeKey, double, EdgeKeyHash> manualSpeedLimit_;
  int threshold_{5};
};

#endif // CONGESTION_MODEL_H
