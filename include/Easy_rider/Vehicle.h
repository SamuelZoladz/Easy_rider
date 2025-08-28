#ifndef VEHICLE_H
#define VEHICLE_H

#include "Graph.h"
#include "Intersection.h"
#include "Road.h"
#include "RouteStrategy.h"
#include <memory>
#include <optional>
#include <utility>
#include <vector>

class CongestionModel; // fwd

/**
 * @class Vehicle
 * @brief Base vehicle with simple longitudinal dynamics and routing policy.
 *
 * Movement model:
 *  - position is tracked along the current edge as a scalar [0 -> length].
 *  - speed integrates with acceleration/braking toward a target speed limited
 *    by both the vehicle's maxSpeed and the edge's effective speed
 *    reported by CongestionModel.
 *  - On reaching the end of an edge, the next edge from @ref route_ is taken.
 *
 * Rerouting:
 *  - When congestion threshold is detected (e.g., on edge entry),
 *    the vehicle may recompute its route using the configured strategy.
 */
class Vehicle {
public:
  virtual ~Vehicle() = default;

  /**
   * @brief Construct a Vehicle.
   * @param graph        World graph reference.
   * @param congestion   Congestion model pointer.
   * @param strategy     Initial routing strategy.
   * @param maxSpeed     Vehicle maximum speed [units/s].
   * @param acceleration Acceleration capability [units/s^2].
   * @param braking      Braking (deceleration) capability [units/s^2].
   */
  Vehicle(const Graph<Intersection, Road> &graph, CongestionModel *congestion,
          std::shared_ptr<RouteStrategy> strategy, double maxSpeed,
          double acceleration, double braking);

  /// @brief Unique vehicle id.
  int id() const { return id_; }

  /// @brief Assign a full route as a sequence of node ids (start -> goal).
  void setRoute(const std::vector<int> &routeIds);

  /// @brief Advance simulation by dt seconds.
  void update(double dt);

  /// @brief Notification that the current edge is congested.
  virtual void onCongestion();

  /// @brief Attempt to recompute route.
  void recomputeRouteIfNeeded();

  /// @brief Replace routing strategy for this vehicle.
  void setStrategy(std::shared_ptr<RouteStrategy> s) {
    strategy_ = std::move(s);
  }

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

  std::optional<RenderState> renderState() const {
    if (route_.empty() || routeIndex_ + 1 >= route_.size())
      return std::nullopt;
    return RenderState{route_[routeIndex_], route_[routeIndex_ + 1],
                       edgeProgress_, currentSpeed_};
  }

protected:
  const Road *findEdge(int fromId, int toId) const;
  void enterEdge(int fromId, int toId);
  void leaveEdge();

  int id_;
  double acceleration_;
  double braking_;
  double maxSpeed_;
  double currentSpeed_{};
  double edgeProgress_{};

  std::pair<int, int> currentEdge_{-1, -1}; // from -> to
  std::vector<int> route_;
  std::size_t routeIndex_{0};

  std::shared_ptr<RouteStrategy> strategy_;
  const Graph<Intersection, Road> *graph_;
  CongestionModel *congestion_;

  // Reroute throttle
  double recomputeCooldown_{3.0}; // seconds
  double sinceRecompute_{1e9};
  bool pendingReroute_{false};
};

#endif // VEHICLE_H
