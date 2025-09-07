#ifndef VEHICLE_H
#define VEHICLE_H

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/RoutingStrategies/RouteStrategy.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"
#include "IDM.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

/**
 * @class Vehicle
 * @brief Base vehicle with longitudinal dynamics (IDM) and routing policy.
 *
 * Movement model:
 *  - Position is tracked along the current edge as a scalar in [0, length].
 *  - Speed is integrated using IDM; free-flow target speed is IDMParams::v0.
 *  - Effective edge speed is limited by the congestion model.
 *  - At an edge end, the next edge from @ref route_ is taken.
 *
 * Rerouting:
 *  - When congestion is detected (e.g., at edge entry), the vehicle may
 *    recompute its route using the configured strategy after a cooldown.
 */

enum class StrategyAlgoritm { Dijkstra, AStar };

class Vehicle {
public:
  virtual ~Vehicle() = default;

  /**
   * @brief Construct a Vehicle with IDM configuration.
   * @param graph        World graph reference.
   * @param congestion   Congestion model pointer (can be null).
   * @param params       Intelligent Driver Model parameters.
   */
  Vehicle(const Graph<Intersection, Road> &graph, CongestionModel *congestion,
          const IDMParams &params);

  /// @brief Unique vehicle id.
  [[nodiscard]] int id() const { return id_; }

  /// @brief Assign a full route as a sequence of node ids (start -> goal).
  void setRoute(const std::vector<int> &routeIds);

  /// @brief Advance simulation by dt seconds.
  void update(double dt);

  /// @brief Notification hook: current edge is congested (may trigger reroute).
  virtual void onCongestion();

  /// @brief Attempt to recompute route if cooldown has elapsed.
  void recomputeRouteIfNeeded();

  /// @brief Replace routing strategy for this vehicle.
  void setStrategy(StrategyAlgoritm algo);

  [[nodiscard]] const std::shared_ptr<RouteStrategy> &strategy() const {
    return strategy_;
  }

  /// @return current node id if exactly at a node, std::nullopt otherwise.
  [[nodiscard]] std::optional<int> currentNodeId() const;

  /// @return goal node id if any.
  [[nodiscard]] std::optional<int> goalId() const;

  struct RenderState {
    int fromId{};
    int toId{};
    double sOnEdge{};
    double currentSpeed{};
  };

  /// @brief Lightweight snapshot for rendering/telemetry.
  [[nodiscard]] std::optional<RenderState> renderState() const {
    if (route_.empty() || routeIndex_ + 1 >= route_.size())
      return std::nullopt;
    return RenderState{route_[routeIndex_], route_[routeIndex_ + 1],
                       edgeProgress_, currentSpeed_};
  }

  /// @brief Override IDM parameters.
  void setIDMParams(const IDMParams &p) { idmParams_ = p; }

  /// @brief Provide leader estimate for this simulation step (edge-aligned).
  /// Call once per tick before update().
  void setLeaderInfo(const LeaderInfo &info) { leader_ = info; }

  /// @brief Clear leader information (e.g., when switching edges).
  void clearLeaderInfo() { leader_.reset(); }

  [[nodiscard]] double currentSpeed() const { return currentSpeed_; }
  [[nodiscard]] double edgeProgress() const { return edgeProgress_; }
  [[nodiscard]] std::pair<int, int> currentEdge() const { return currentEdge_; }

  // Convenience accessors mapped to IDM parameters.
  [[nodiscard]] double maxSpeed() const { return idmParams_.v0; }
  [[nodiscard]] double accelLimit() const { return idmParams_.a; }
  [[nodiscard]] double brakeLimit() const { return idmParams_.b; }

  [[nodiscard]] bool hasArrived() const noexcept;

  /// @brief Callback invoked after a re-route is applied:
  ///        (vehId, oldETA, newETA).
  void setOnRerouteApplied(std::function<void(int, double, double)> cb) {
    onRerouteApplied_ = std::move(cb);
  }

protected:
  /// @brief Find a road by (fromId -> toId). Returns nullptr if missing.
  [[nodiscard]] const Road *findEdge(int fromId, int toId) const;

  /// @brief Enter a new edge; resets progress and updates congestion counters.
  void enterEdge(int fromId, int toId);

  /// @brief Leave the current edge; updates congestion counters.
  void leaveEdge();

  int id_{};
  double currentSpeed_{};                   ///< Current speed along edge.
  double edgeProgress_{};                   ///< Position along current edge.
  std::pair<int, int> currentEdge_{-1, -1}; // from -> to

  std::vector<int> route_;
  std::size_t routeIndex_{0};
  std::shared_ptr<RouteStrategy> strategy_{};

  const Graph<Intersection, Road> *graph_{};
  CongestionModel *congestion_{};

  double recomputeCooldown_{3.0}; // seconds
  double sinceRecompute_{1e9};
  bool pendingReroute_{false};

  IDMParams idmParams_{};
  std::optional<LeaderInfo> leader_{};

  [[nodiscard]] double estimateRemainingETA(const std::vector<int> &path,
                                            std::size_t routeIndex,
                                            double sOnEdge) const;

  std::function<void(int, double, double)> onRerouteApplied_{};
};

#endif // VEHICLE_H
