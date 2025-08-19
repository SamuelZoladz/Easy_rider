#include "Easy_rider/Vehicle.h"
#include "Easy_rider/CongestionModel.h"
#include <algorithm>
#include <cassert>

namespace {
static int s_nextVehicleId = 1;
}

Vehicle::Vehicle(const Graph<Intersection, Road> &graph,
                 CongestionModel *congestion,
                 std::shared_ptr<RouteStrategy> strategy, double maxSpeed,
                 double acceleration, double braking)
    : id_(s_nextVehicleId++), acceleration_(acceleration), braking_(braking),
      maxSpeed_(maxSpeed), strategy_(std::move(strategy)), graph_(&graph),
      congestion_(congestion) {}

void Vehicle::setRoute(const std::vector<int> &routeIds) {
  route_ = routeIds;
  routeIndex_ = 0;
  edgeProgress_ = 0.0;
  currentSpeed_ = 0.0;
  if (route_.size() >= 2)
    enterEdge(route_[0], route_[1]);
  else
    currentEdge_ = {-1, -1};
}

std::optional<int> Vehicle::currentNodeId() const {
  // Exactly at a node if not on an edge or progress == 0 at edge start
  if (currentEdge_.first < 0)
    return route_.empty() ? std::optional<int>{}
                          : std::optional<int>{route_[0]};
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
  if (route_.size() >= 1)
    return route_.back();
  return std::nullopt;
}

const Road *Vehicle::findEdge(int fromId, int toId) const {
  try {
    int uIdx = static_cast<int>(graph_->indexOfId(fromId));
    int vIdx = static_cast<int>(graph_->indexOfId(toId));
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
  if (congestion_)
    congestion_->onEnterEdge(currentEdge_);
}

void Vehicle::leaveEdge() {
  if (congestion_ && currentEdge_.first >= 0)
    congestion_->onExitEdge(currentEdge_);
  currentEdge_ = {-1, -1};
}

void Vehicle::onCongestion() { pendingReroute_ = true; }

void Vehicle::recomputeRouteIfNeeded() {
  if (!strategy_)
    return;
  if (sinceRecompute_ < recomputeCooldown_)
    return;
  auto goal = goalId();
  if (!goal)
    return;
  int startId;
  if (auto node = currentNodeId())
    startId = *node;
  else
    startId = currentEdge_.second;

  auto newRoute = strategy_->computeRoute(startId, *goal, *graph_);
  if (newRoute.size() >= 2) {
    if (auto node = currentNodeId()) {
      setRoute(newRoute);
      pendingReroute_ = false;
      sinceRecompute_ = 0.0;
    } else {
      route_.assign(newRoute.begin(), newRoute.end());
      routeIndex_ = 0;
      pendingReroute_ = false;
      sinceRecompute_ = 0.0;
    }
  }
}

void Vehicle::update(double dt) {
  sinceRecompute_ += dt;
  if (route_.size() < 2 || routeIndex_ >= route_.size() - 1)
    return;

  const Road *edge = findEdge(currentEdge_.first, currentEdge_.second);
  if (!edge)
    return;

  const double vEff = congestion_ ? congestion_->effectiveSpeed(*edge)
                                  : static_cast<double>(edge->getMaxSpeed());
  const double vTarget = std::min(maxSpeed_, vEff);

  if (currentSpeed_ < vTarget)
    currentSpeed_ = std::min(vTarget, currentSpeed_ + acceleration_ * dt);
  else if (currentSpeed_ > vTarget)
    currentSpeed_ = std::max(vTarget, currentSpeed_ - braking_ * dt);

  const double dist = currentSpeed_ * dt;
  edgeProgress_ += dist;

  if (edgeProgress_ + 1e-9 >= edge->getLength()) {
    leaveEdge();
    ++routeIndex_;
    if (routeIndex_ >= route_.size() - 1) {
      currentSpeed_ = 0.0; // arrived
      return;
    }
    enterEdge(route_[routeIndex_], route_[routeIndex_ + 1]);

    // Congestion check at edge entry.
    const Road *newEdge = findEdge(currentEdge_.first, currentEdge_.second);
    if (newEdge && congestion_ && congestion_->isCongested(*newEdge))
      onCongestion();

    if (pendingReroute_)
      recomputeRouteIfNeeded();
  }
}
