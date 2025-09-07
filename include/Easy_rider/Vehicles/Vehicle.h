#ifndef VEHICLE_H
#define VEHICLE_H

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/RoutingStrategies/RouteStrategy.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"
#include "IDM.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

/**
 * @class Vehicle
 * @brief Base vehicle with longitudinal dynamics (IDM) and routing policy.
 *
 * Movement model:
 *  - Position is tracked along the current edge as a scalar [0 -> length].
 *  - Speed is integrated using IDM; the free-flow target speed is @ref
 * IDMParams::v0.
 *  - Effective speed on an edge is limited by the congestion model.
 *  - On reaching the end of an edge, the next edge from @ref route_ is taken.
 *
 * Rerouting:
 *  - When congestion is detected (e.g., on edge entry), the vehicle may
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
  int id() const { return id_; }

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

  const std::shared_ptr<RouteStrategy> &strategy() const { return strategy_; }

  /// @brief @return current node id if exactly at a node, std::nullopt
  /// otherwise.
  std::optional<int> currentNodeId() const;

  /// @brief @return goal node id if any.
  std::optional<int> goalId() const;

  struct RenderState {
    int fromId{};
    int toId{};
    double sOnEdge{};
    double currentSpeed{};
  };

  /// @brief Lightweight snapshot for rendering.
  std::optional<RenderState> renderState() const {
    if (route_.empty() || routeIndex_ + 1 >= route_.size())
      return std::nullopt;
    return RenderState{route_[routeIndex_], route_[routeIndex_ + 1],
                       edgeProgress_, currentSpeed_};
  }

  /// @brief Override IDM parameters.
  void setIDMParams(const IDMParams &p) { idmParams_ = p; }

  /// @brief Provide leader estimate for this simulation step (edge-aligned).
  /// Use gap (distance to leader bumper) and leader speed; call once per tick
  /// before update().
  void setLeaderInfo(const LeaderInfo &info) { leader_ = info; }

  /// @brief Clear leader information (e.g., when switching edges).
  void clearLeaderInfo() { leader_.reset(); }

  double currentSpeed() const { return currentSpeed_; }
  double edgeProgress() const { return edgeProgress_; }
  std::pair<int, int> currentEdge() const { return currentEdge_; }

  /// @brief Convenience accessors mapped to IDM parameters.
  double maxSpeed() const { return idmParams_.v0; }
  double accelLimit() const { return idmParams_.a; }
  double brakeLimit() const { return idmParams_.b; }
  bool hasArrived() const noexcept;
  void setOnRerouteApplied(std::function<void(int, double, double)> cb) {
    onRerouteApplied_ = std::move(cb);
  }

protected:
  const Road *findEdge(int fromId, int toId) const;
  void enterEdge(int fromId, int toId);
  void leaveEdge();

  int id_{};
  double currentSpeed_{}; ///< current speed along edge
  double edgeProgress_{}; ///< curvilinear position along current edge

  std::pair<int, int> currentEdge_{-1, -1}; // from -> to
  std::vector<int> route_;
  std::size_t routeIndex_{0};

  std::shared_ptr<RouteStrategy> strategy_;
  const Graph<Intersection, Road> *graph_{};
  CongestionModel *congestion_{};

  // Reroute throttle
  double recomputeCooldown_{3.0}; // seconds
  double sinceRecompute_{1e9};
  bool pendingReroute_{false};
  IDMParams idmParams_{};
  std::optional<LeaderInfo> leader_;
  double estimateRemainingETA(const std::vector<int> &path,
                              std::size_t routeIndex, double sOnEdge) const;

  std::function<void(int, double, double)> onRerouteApplied_{};
};

#endif // VEHICLE_H
