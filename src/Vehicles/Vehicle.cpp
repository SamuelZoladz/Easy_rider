#include "Easy_rider/Vehicles/Vehicle.h"

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/Vehicles/IDM.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

#include "Easy_rider/RoutingStrategies/AStarStrategy.h"
#include "Easy_rider/RoutingStrategies/DijkstraStrategy.h"

namespace {
int s_nextVehicleId = 1;
constexpr double kTiny = 1e-9;
constexpr double kDtFloor = 1e-3;
} // namespace

Vehicle::Vehicle(const Graph<Intersection, Road> &graph,
                 CongestionModel *congestion, const IDMParams &params)
    : id_(s_nextVehicleId++), graph_(&graph), congestion_(congestion),
      idmParams_(params) {}

void Vehicle::setStrategy(StrategyAlgoritm algo) {
  using EdgeTimeFn = std::function<double(const Road &)>;
  EdgeTimeFn timeFn = [this](const Road &e) {
    assert(congestion_ && "Vehicle must have a congestion model");
    return congestion_->edgeTime(e, idmParams_.v0);
  };

  switch (algo) {
  case StrategyAlgoritm::AStar:
    strategy_ = std::make_shared<AStarStrategy>(std::move(timeFn));
    break;
  case StrategyAlgoritm::Dijkstra:
    strategy_ = std::make_shared<DijkstraStrategy>(std::move(timeFn));
    break;
  }

  // Trigger a recompute soon after strategy change.
  pendingReroute_ = true;
  sinceRecompute_ = recomputeCooldown_;
}

void Vehicle::setRoute(const std::vector<int> &routeIds) {
  route_ = routeIds;
  routeIndex_ = 0;
  edgeProgress_ = 0.0;
  currentSpeed_ = 0.0;

  if (route_.size() >= 2) {
    enterEdge(route_[0], route_[1]);
  } else {
    currentEdge_ = {-1, -1};
  }
}

std::optional<int> Vehicle::currentNodeId() const {
  // Exactly at a node if not on an edge or progress == 0 at edge start/end.
  if (currentEdge_.first < 0)
    return route_.empty() ? std::nullopt : std::optional<int>{route_[0]};

  const Road *e = findEdge(currentEdge_.first, currentEdge_.second);
  if (!e)
    return std::nullopt;

  if (edgeProgress_ <= 0.0)
    return currentEdge_.first;
  if (edgeProgress_ >= e->getLength())
    return currentEdge_.second;
  return std::nullopt;
}

std::optional<int> Vehicle::goalId() const {
  if (!route_.empty())
    return route_.back();
  return std::nullopt;
}

const Road *Vehicle::findEdge(int fromId, int toId) const {
  try {
    const int uIdx = static_cast<int>(graph_->indexOfId(fromId));
    const int vIdx = static_cast<int>(graph_->indexOfId(toId));
    for (const auto &nbr : graph_->outgoing(uIdx)) {
      if (nbr.first == vIdx)
        return &nbr.second.get();
    }
  } catch (...) {
    return nullptr;
  }
  return nullptr;
}

void Vehicle::enterEdge(int fromId, int toId) {
  currentEdge_ = {fromId, toId};
  edgeProgress_ = 0.0;
  leader_.reset();

  if (congestion_)
    congestion_->onEnterEdge(currentEdge_);

  // If entering a slower edge, cap the current speed to local effective limit.
  if (const Road *e = findEdge(fromId, toId)) {
    const double vEff = congestion_ ? congestion_->effectiveSpeed(*e)
                                    : static_cast<double>(e->getMaxSpeed());
    const double vCap = std::min(idmParams_.v0, vEff);
    if (currentSpeed_ > vCap)
      currentSpeed_ = vCap;
  }
}

void Vehicle::leaveEdge() {
  if (congestion_ && currentEdge_.first >= 0)
    congestion_->onExitEdge(currentEdge_);
  currentEdge_ = {-1, -1};
  leader_.reset();
}

bool Vehicle::hasArrived() const noexcept {
  const auto g = goalId();
  const auto n = currentNodeId();
  const bool atEndIdx =
      (route_.size() >= 2 && routeIndex_ >= route_.size() - 1);
  return g.has_value() && n.has_value() && atEndIdx && (*g == *n);
}

double Vehicle::estimateRemainingETA(const std::vector<int> &path,
                                     std::size_t idx, double sOnEdge) const {
  if (!graph_ || !congestion_ || path.size() < 2 || idx >= path.size() - 1) {
    return 0.0;
  }

  double eta = 0.0;

  // Remaining segment on the current edge.
  if (const Road *e = findEdge(path[idx], path[idx + 1])) {
    const double len = std::max(0.0, e->getLength() - std::max(0.0, sOnEdge));
    const double v = std::max(kTiny, congestion_->effectiveSpeed(*e));
    eta += len / v;
  }

  // Full lengths of subsequent edges.
  for (std::size_t i = idx + 1; i + 1 < path.size(); ++i) {
    if (const Road *e = findEdge(path[i], path[i + 1])) {
      const double len = std::max(0.0, e->getLength());
      const double v = std::max(kTiny, congestion_->effectiveSpeed(*e));
      eta += len / v;
    }
  }
  return eta;
}

void Vehicle::onCongestion() { pendingReroute_ = true; }

void Vehicle::recomputeRouteIfNeeded() {
  if (!strategy_ || sinceRecompute_ < recomputeCooldown_)
    return;

  const auto goal = goalId();
  if (!goal)
    return;

  // If mid-edge, plan from the next node; otherwise from the current node.
  const int startId = currentNodeId().value_or(currentEdge_.second);
  const auto newRoute = strategy_->computeRoute(startId, *goal, *graph_);
  if (newRoute.size() < 2)
    return;

  if (newRoute == route_) {
    pendingReroute_ = false;
    return;
  }

  // Compare ETAs from current situation vs. new route.
  const std::vector<int> oldRoute = route_;
  const std::size_t oldIdx = routeIndex_;
  const double oldS = currentNodeId() ? 0.0 : std::max(0.0, edgeProgress_);
  const double oldETA = estimateRemainingETA(oldRoute, oldIdx, oldS);

  double newETA = 0.0;

  if (currentNodeId()) {
    // At a node: switch immediately to the new route; preserve speed.
    const double vKeep = currentSpeed_;
    setRoute(newRoute);
    currentSpeed_ = vKeep;
    newETA = estimateRemainingETA(newRoute, /*idx=*/0, /*sOnEdge=*/0.0);
  } else {
    // Mid-edge: keep traversing the current edge, then follow newRoute.
    route_.assign(newRoute.begin(), newRoute.end());
    routeIndex_ = 0;
    newETA = estimateRemainingETA(newRoute, /*idx=*/0,
                                  /*sOnEdge=*/std::max(0.0, edgeProgress_));
  }

  pendingReroute_ = false;
  sinceRecompute_ = 0.0;

  if (onRerouteApplied_)
    onRerouteApplied_(id_, oldETA, newETA);
}

void Vehicle::update(double dt) {
  sinceRecompute_ += dt;

  if (dt <= 0.0)
    return;
  if (route_.size() < 2 || routeIndex_ >= route_.size() - 1)
    return;

  const Road *edge = findEdge(currentEdge_.first, currentEdge_.second);
  if (!edge)
    return;

  // Effective speed on current edge (congestion-aware), capped by vehicle v0.
  const double vEffCurRaw = congestion_
                                ? congestion_->effectiveSpeed(*edge)
                                : static_cast<double>(edge->getMaxSpeed());
  double v0_local = std::min(idmParams_.v0, vEffCurRaw);

  // Lookahead: plan to match the next edge's cap by the end of this edge.
  if (routeIndex_ + 1 < static_cast<int>(route_.size()) - 1) {
    const int nextFrom = route_[routeIndex_ + 1];
    const int nextTo = route_[routeIndex_ + 2];
    if (const Road *nextEdge = findEdge(nextFrom, nextTo)) {
      const double vEffNextRaw =
          congestion_ ? congestion_->effectiveSpeed(*nextEdge)
                      : static_cast<double>(nextEdge->getMaxSpeed());
      const double v0_next = std::min(idmParams_.v0, vEffNextRaw);

      const double s_rem = std::max(0.0, edge->getLength() - edgeProgress_);
      const double bPlan = std::max(0.1, idmParams_.b);

      // Kinematic cap to ensure we can reach v0_next by the edge end.
      double vcap =
          std::sqrt(std::max(0.0, v0_next * v0_next + 2.0 * bPlan * s_rem));
      vcap += kTiny;
      v0_local = std::min(v0_local, vcap);
    }
  }

  const double v0 = v0_local;

  // IDM acceleration based on leader info (if any).
  double accel = 0.0;
  if (leader_ && leader_->present) {
    const double gap = std::max(0.0, leader_->gap);
    const double dv = std::max(0.0, currentSpeed_ - leader_->leaderSpeed);

    accel = idm_accel(currentSpeed_, v0, gap, dv, idmParams_);

    const double aMax = std::max(0.1, idmParams_.a);
    const double bMax = std::max(0.1, idmParams_.b);
    if (accel > aMax)
      accel = aMax;
    if (accel < -bMax)
      accel = -bMax;
  } else {
    // Free road: relax toward v0 within a single step.
    if (currentSpeed_ < v0) {
      accel =
          std::min(idmParams_.a, (v0 - currentSpeed_) / std::max(kDtFloor, dt));
    } else if (currentSpeed_ > v0) {
      accel = -std::min(idmParams_.b,
                        (currentSpeed_ - v0) / std::max(kDtFloor, dt));
    } else {
      accel = 0.0;
    }
  }

  // Integrate speed with clamping to [0, v0] (if accelerating).
  const double vNext = currentSpeed_ + accel * dt;
  currentSpeed_ = (accel >= 0.0) ? std::min(vNext, v0) : std::max(0.0, vNext);

  // Advance along the edge.
  edgeProgress_ += currentSpeed_ * dt;

  // Edge transition.
  if (edgeProgress_ + kTiny >= edge->getLength()) {
    leaveEdge();
    ++routeIndex_;
    if (routeIndex_ >= route_.size() - 1) {
      currentSpeed_ = 0.0; // Arrived
      return;
    }
    enterEdge(route_[routeIndex_], route_[routeIndex_ + 1]);

    // If the new edge is congested, mark for re-route consideration.
    const Road *newEdge = findEdge(currentEdge_.first, currentEdge_.second);
    if (newEdge && congestion_ &&
        congestion_->effectiveSpeed(*newEdge) < newEdge->getMaxSpeed()) {
      onCongestion();
    }

    if (pendingReroute_) {
      recomputeRouteIfNeeded();
    }
  }
}
