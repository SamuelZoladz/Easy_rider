#include "Easy_rider/Simulation/Simulation.h"
#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/RoutingStrategies/RouteStrategy.h"
#include "Easy_rider/Vehicles/Car.h"
#include "Easy_rider/Vehicles/IDM.h"
#include "Easy_rider/Vehicles/Truck.h"
#include "Easy_rider/Vehicles/Vehicle.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <utility>

#include "Easy_rider/Parameters/Parameters.h"

// ===== Debug switches =====
#ifndef SIM_DBG
#define SIM_DBG 0
#endif

#if SIM_DBG
#define SLOG_INIT()                                                            \
  std::cout.setf(std::ios::fixed);                                             \
  std::cout << std::setprecision(3)
#define SLOG(MSG)                                                              \
  do {                                                                         \
    SLOG_INIT();                                                               \
    std::cout << "[Sim] " << MSG << std::endl;                                 \
  } while (0)
#else
#define SLOG_INIT()                                                            \
  do {                                                                         \
  } while (0)
#define SLOG(MSG)                                                              \
  do {                                                                         \
  } while (0)
#endif
// ==========================

Simulation::~Simulation() = default;

void Simulation::update(double dt) {
  if (!running_ || paused_)
    return;

  const double step = dt * Parameters::simulationSpeed();
  simTime_ += step;
  SLOG("tick dt=" << dt << " timeScale=" << timeScale_ << " step=" << step
                  << " vehicles=" << vehicles_.size());

  // Build per-edge ordered lists (vehicles sorted by progress on each edge).
  std::unordered_map<EdgeKey, std::vector<std::pair<double, Vehicle *>>,
                     EdgeKeyHash>
      lanes;
  lanes.reserve(vehicles_.size());
  for (auto &up : vehicles_) {
    Vehicle *v = up.get();
    auto e = v->currentEdge();
    if (e.first < 0)
      continue; // at node or no route
    lanes[e].push_back({v->edgeProgress(), v});
  }

  // Sort by progress along each edge.
  for (auto &kv : lanes) {
    auto &vec = kv.second;
    std::sort(vec.begin(), vec.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });

    SLOG("edge " << kv.first.first << " -> " << kv.first.second
                 << " count=" << vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i) {
      SLOG("  [" << i << "] s=" << vec[i].first << " ptr=" << vec[i].second);
    }
  }

  // Feed leader info to vehicles for IDM.
  for (auto &kv : lanes) {
    const EdgeKey key = kv.first;

    // Resolve edge pointer (from -> to).
    const Road *edgePtr = nullptr;
    try {
      int u = static_cast<int>(graph_.indexOfId(key.first));
      int v = static_cast<int>(graph_.indexOfId(key.second));
      for (const auto &nbr : graph_.outgoing(u)) {
        if (nbr.first == v) {
          edgePtr = &nbr.second.get();
          break;
        }
      }
    } catch (...) {
      edgePtr = nullptr;
    }
    if (!edgePtr) {
      SLOG("cannot resolve edge " << key.first << " -> " << key.second);
      continue;
    }

    auto &vec = kv.second;
    for (std::size_t i = 0; i < vec.size(); ++i) {
      Vehicle *me = vec[i].second;
      me->clearLeaderInfo();
      LeaderInfo li;
      if (i + 1 < vec.size()) {
        Vehicle *lead = vec[i + 1].second;
        li.present = true;
        li.gap = std::max(0.0, lead->edgeProgress() - me->edgeProgress());
        li.leaderSpeed = lead->currentSpeed();
        SLOG("leader on " << key.first << " -> " << key.second
                          << " gap=" << li.gap << " v_lead=" << li.leaderSpeed
                          << " me_ptr=" << me << " lead_ptr=" << lead);
      } else {
        li.present = false;
        li.gap = std::max(0.0, edgePtr->getLength() - me->edgeProgress());
        li.leaderSpeed = 0.0;
        SLOG("open road on " << key.first << " -> " << key.second
                             << " gap_to_end=" << li.gap << " me_ptr=" << me);
      }
      me->setLeaderInfo(li);
    }
  }

  for (auto &up : vehicles_) {
    up->update(step);
  }
}

int Simulation::spawnVehicleCar(int startId, int goalId,
                                StrategyAlgoritm algo) {
  auto veh = std::make_unique<Car>(graph_, &congestion_);
  int id = static_cast<int>(vehicles_.size());
  vehicles_.push_back(std::move(veh));
  SLOG("spawnVehicleCar idx=" << id << " start=" << startId
                              << " goal=" << goalId);

  vehicles_[static_cast<std::size_t>(id)]->setStrategy(algo);
  ensureInitialRoutes(id, startId, goalId);
  return id;
}

int Simulation::spawnVehicleTruck(int startId, int goalId,
                                  StrategyAlgoritm algo) {
  auto veh = std::make_unique<Truck>(graph_, &congestion_);
  int id = static_cast<int>(vehicles_.size());
  vehicles_.push_back(std::move(veh));
  SLOG("spawnVehicleTruck idx=" << id << " start=" << startId
                                << " goal=" << goalId);

  vehicles_[static_cast<std::size_t>(id)]->setStrategy(algo);
  ensureInitialRoutes(id, startId, goalId);
  return id;
}

void Simulation::setStrategyForAll(StrategyAlgoritm algo) {
  SLOG("setStrategyForAll(algo)");
  for (auto &v : vehicles_)
    v->setStrategy(algo);
}

void Simulation::ensureInitialRoutes(int vehIdx, int startId, int goalId) {
  assert(vehIdx >= 0 && static_cast<std::size_t>(vehIdx) < vehicles_.size());
  auto &veh = vehicles_[static_cast<std::size_t>(vehIdx)];
  auto route = veh->strategy()->computeRoute(startId, goalId, graph_);
  SLOG("ensureInitialRoutes vehIdx=" << vehIdx << " start=" << startId
                                     << " goal=" << goalId
                                     << " routeSize=" << route.size());
  veh->setRoute(route);
}

std::vector<Simulation::SimSnapshotItem> Simulation::snapshot() const {
  std::vector<SimSnapshotItem> out;
  out.reserve(vehicles_.size());
  int idx = 0;
  for (const auto &ptr : vehicles_) {
    if (auto rs = ptr->renderState()) {
      out.push_back(SimSnapshotItem{idx, rs->fromId, rs->toId, rs->sOnEdge,
                                    rs->currentSpeed});
    }
    ++idx;
  }
  return out;
}

double Simulation::getSimTime() const noexcept { return simTime_; }

double Simulation::averageSpeed() const noexcept {
  double sum = 0.0;
  std::size_t count = 0;

  for (const auto &up : vehicles_) {
    if (up->renderState()) {
      sum += up->currentSpeed();
      ++count;
    }
  }

  if (count == 0)
    return 0.0;

#if SIM_DBG
  SLOG("averageSpeed count=" << count
                             << " avg=" << (sum / static_cast<double>(count)));
#endif

  return sum / static_cast<double>(count);
}
