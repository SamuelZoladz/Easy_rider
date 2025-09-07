#include "Easy_rider/Vehicles/Vehicle.h"

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/Vehicles/IDM.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>

namespace {
static int s_nextVehicleId = 1;
}

// ===== Debug switches =====
#ifndef VEH_DBG
#define VEH_DBG 0
#endif

#if VEH_DBG
#define VLOG_INIT()                                                            \
  std::cout.setf(std::ios::fixed);                                             \
  std::cout << std::setprecision(3)
#define VLOG(MSG)                                                              \
  do {                                                                         \
    VLOG_INIT();                                                               \
    std::cout << "[Vehicle#" << id_ << "] " << MSG << std::endl;               \
  } while (0)
#else
#define VLOG_INIT()                                                            \
  do {                                                                         \
  } while (0)
#define VLOG(MSG)                                                              \
  do {                                                                         \
  } while (0)
#endif
// ==========================

Vehicle::Vehicle(const Graph<Intersection, Road> &graph,
                 CongestionModel *congestion,
                 std::shared_ptr<RouteStrategy> strategy,
                 const IDMParams &params)
    : id_(s_nextVehicleId++), strategy_(std::move(strategy)), graph_(&graph),
      congestion_(congestion), idmParams_(params) {
  VLOG("CTOR IDM v0=" << idmParams_.v0 << " a=" << idmParams_.a
                      << " b=" << idmParams_.b);
}

void Vehicle::setRoute(const std::vector<int> &routeIds) {
  route_ = routeIds;
  routeIndex_ = 0;
  edgeProgress_ = 0.0;
  currentSpeed_ = 0.0;

  VLOG("setRoute size=" << route_.size() << " route: ");
  if (!route_.empty()) {
    std::cout << "  [Vehicle#" << id_ << "] route nodes: ";
    for (std::size_t i = 0; i < route_.size(); ++i) {
      if (i)
        std::cout << " -> ";
      std::cout << route_[i];
    }
    std::cout << std::endl;
  }

  if (route_.size() >= 2)
    enterEdge(route_[0], route_[1]);
  else {
    currentEdge_ = {-1, -1};
    VLOG("no drivable edge (route too short)");
  }
}

std::optional<int> Vehicle::currentNodeId() const {
  // Exactly at a node if not on an edge or progress == 0 at edge start
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
  leader_.reset();

  double len = -1.0;
  if (const Road *e = findEdge(fromId, toId))
    len = e->getLength();
  VLOG("enterEdge " << fromId << " -> " << toId << " len=" << len);

  if (congestion_)
    congestion_->onEnterEdge(currentEdge_);

  if (const Road *e = findEdge(fromId, toId)) {
    const double vEff = congestion_ ? congestion_->effectiveSpeed(*e)
                                    : static_cast<double>(e->getMaxSpeed());
    const double vCap = std::min(idmParams_.v0, vEff);
    if (currentSpeed_ > vCap) {
      VLOG("edge entry cap: v=" << currentSpeed_ << " -> " << vCap << " on "
                                << fromId << " -> " << toId);
      currentSpeed_ = vCap;
    }
  }
}

void Vehicle::leaveEdge() {
  double len = -1.0;
  if (currentEdge_.first >= 0) {
    if (const Road *e = findEdge(currentEdge_.first, currentEdge_.second))
      len = e->getLength();
  }
  VLOG("leaveEdge " << currentEdge_.first << " -> " << currentEdge_.second
                    << " progress=" << edgeProgress_ << "/" << len);

  if (congestion_ && currentEdge_.first >= 0)
    congestion_->onExitEdge(currentEdge_);
  currentEdge_ = {-1, -1};
  leader_.reset();
}

void Vehicle::onCongestion() {
  VLOG("onCongestion() flag pendingReroute_=true");
  pendingReroute_ = true;
}

void Vehicle::recomputeRouteIfNeeded() {
  VLOG("recomputeRouteIfNeeded since=" << sinceRecompute_
                                       << " cooldown=" << recomputeCooldown_);
  if (!strategy_ || sinceRecompute_ < recomputeCooldown_) {
    VLOG("skip recompute (no strategy or cooldown not reached)");
    return;
  }
  auto goal = goalId();
  if (!goal) {
    VLOG("skip recompute (no goal)");
    return;
  }

  int startId = currentNodeId().value_or(currentEdge_.second);
  auto newRoute = strategy_->computeRoute(startId, *goal, *graph_);
  VLOG("strategy_->computeRoute(" << startId << " -> " << *goal
                                  << ") size=" << newRoute.size());

  if (newRoute.size() < 2) {
    VLOG("newRoute too short, abort recompute");
    return;
  }

  if (currentNodeId()) {
    if (currentEdge_.first >= 0)
      leaveEdge();

    double vKeep = currentSpeed_;
    setRoute(newRoute);
    currentSpeed_ = vKeep;
    VLOG("recompute applied (at node), kept v=" << vKeep);
  } else {
    route_.assign(newRoute.begin(), newRoute.end());
    routeIndex_ = 0;
    VLOG("recompute applied (on edge), routeIndex reset to 0");
  }

  pendingReroute_ = false;
  sinceRecompute_ = 0.0;
}

void Vehicle::update(double dt) {
  sinceRecompute_ += dt;

  if (route_.size() < 2 || routeIndex_ >= route_.size() - 1) {
    VLOG("update early exit: route.size=" << route_.size()
                                          << " routeIndex=" << routeIndex_);
    return;
  }

  const Road *edge = findEdge(currentEdge_.first, currentEdge_.second);
  if (!edge) {
    VLOG("no current edge found for " << currentEdge_.first << " -> "
                                      << currentEdge_.second);
    return;
  }

  VLOG("update dt=" << dt << " idx=" << routeIndex_
                    << " edge=" << currentEdge_.first << " -> "
                    << currentEdge_.second << " s=" << edgeProgress_ << "/"
                    << edge->getLength() << " v=" << currentSpeed_);

  const double vEffCurRaw = congestion_
                                ? congestion_->effectiveSpeed(*edge)
                                : static_cast<double>(edge->getMaxSpeed());
  double v0_local = std::min(idmParams_.v0, vEffCurRaw);

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

      double vcap =
          std::sqrt(std::max(0.0, v0_next * v0_next + 2.0 * bPlan * s_rem));
      vcap += 1e-6;
      const double v0_before = v0_local;
      v0_local = std::min(v0_local, vcap);
      VLOG("anticipatory cap: v0_cur=" << v0_before << " v0_next=" << v0_next
                                       << " vcap=" << vcap
                                       << " s_rem=" << s_rem);
    }
  }

  const double v0 = v0_local;

  VLOG("limits vEffCur=" << vEffCurRaw << " edgeLimit=" << edge->getMaxSpeed()
                         << " v0=" << v0);

  double accel = 0.0;
  double gap = std::numeric_limits<double>::infinity();
  double dv = 0.0; // closing speed = v - v_leader (>=0)

  if (leader_ && leader_->present) {
    gap = std::max(0.0, leader_->gap);
    dv = std::max(0.0, currentSpeed_ - leader_->leaderSpeed);

    accel = idm_accel(currentSpeed_, v0, gap, dv, idmParams_);

    const double aMax = std::max(0.1, idmParams_.a);
    const double bMax = std::max(0.1, idmParams_.b);
    if (accel > aMax)
      accel = aMax;
    if (accel < -bMax)
      accel = -bMax;

    VLOG("IDM gap=" << (std::isinf(gap) ? 1e9 : gap) << " dv=" << dv
                    << " accel(clamped)=" << accel);
  } else {
    if (currentSpeed_ < v0)
      accel = std::min(idmParams_.a, (v0 - currentSpeed_) / std::max(1e-3, dt));
    else if (currentSpeed_ > v0)
      accel =
          -std::min(idmParams_.b, (currentSpeed_ - v0) / std::max(1e-3, dt));
    else
      accel = 0.0;
    VLOG("LEGACY accel=" << accel);
  }

  const double vBefore = currentSpeed_;
  const double vNext = currentSpeed_ + accel * dt;
  if (accel >= 0.0) {
    if (vNext > v0)
      VLOG("cap on accel: vNext=" << vNext << " -> v0=" << v0);
    currentSpeed_ = std::min(vNext, v0);
  } else {
    currentSpeed_ = std::max(0.0, vNext);
  }

  const double dist = currentSpeed_ * dt;
  const double sBefore = edgeProgress_;
  edgeProgress_ += dist;

  VLOG("integrated v: " << vBefore << " -> " << currentSpeed_
                        << " dist=" << dist << " s: " << sBefore << " -> "
                        << edgeProgress_);

  if (edgeProgress_ + 1e-9 >= edge->getLength()) {
    VLOG("edge end reached at s=" << edgeProgress_
                                  << " len=" << edge->getLength());
    leaveEdge();
    ++routeIndex_;
    if (routeIndex_ >= route_.size() - 1) {
      const int goal = goalId().value_or(-1);
      VLOG("route finished. node=" << currentEdge_.second << " goal=" << goal
                                   << " stopping with v=0");
      currentSpeed_ = 0.0; // arrived
      return;
    }
    VLOG("switch to next edge: " << route_[routeIndex_] << " -> "
                                 << route_[routeIndex_ + 1]);
    enterEdge(route_[routeIndex_], route_[routeIndex_ + 1]);

    // Congestion check at edge entry.
    const Road *newEdge = findEdge(currentEdge_.first, currentEdge_.second);
    if (newEdge && congestion_ &&
        congestion_->effectiveSpeed(*newEdge) < newEdge->getMaxSpeed()) {
      VLOG("entry congestion detected on edge " << currentEdge_.first << " -> "
                                                << currentEdge_.second);
      onCongestion();
    }

    if (pendingReroute_) {
      VLOG("pendingReroute_==true, trying recompute");
      recomputeRouteIfNeeded();
    }
  }
}
