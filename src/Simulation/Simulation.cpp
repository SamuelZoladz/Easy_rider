#include "Easy_rider/Simulation/Simulation.h"

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/Parameters/Parameters.h"
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
#include <vector>

namespace {

// Map key: directed edge -> ordered list of (progress, vehicle*)
using Lanes =
    std::unordered_map<EdgeKey, std::vector<std::pair<double, Vehicle *>>,
                       EdgeKeyHash>;

/**
 * @brief Resolve a Road* for a directed edge (fromId -> toId).
 * @return Pointer to Road or nullptr if not found.
 */
const Road *findEdge(const Graph<Intersection, Road> &graph,
                     const EdgeKey &key) {
  try {
    const int u = static_cast<int>(graph.indexOfId(key.first));
    const int v = static_cast<int>(graph.indexOfId(key.second));
    for (const auto &nbr : graph.outgoing(u)) {
      if (nbr.first == v) {
        return &nbr.second.get();
      }
    }
  } catch (...) {
    // fallthrough
  }
  return nullptr;
}

/**
 * @brief Shared implementation for spawning vehicle types.
 * @tparam T Concrete Vehicle type (Car, Truck, ...).
 */
template <typename T>
int spawnVehicleOf(std::vector<std::unique_ptr<Vehicle>> &vehicles,
                   Graph<Intersection, Road> &graph,
                   CongestionModel &congestion, int startId, int goalId,
                   StrategyAlgoritm algo, std::size_t &rerouteCount,
                   double &rerouteSavedTime) {
  auto veh = std::make_unique<T>(graph, &congestion);
  const int id = static_cast<int>(vehicles.size());
  vehicles.emplace_back(std::move(veh));

  vehicles[static_cast<std::size_t>(id)]->setStrategy(algo);
  vehicles[static_cast<std::size_t>(id)]->setOnRerouteApplied(
      [&rerouteCount, &rerouteSavedTime](int /*vehId*/, double oldETA,
                                         double newETA) {
        ++rerouteCount;
        if (oldETA > newETA)
          rerouteSavedTime += (oldETA - newETA);
      });

  return id;
}

} // namespace

Simulation::~Simulation() = default;

void Simulation::update(double dt) {
  if (!running_ || paused_)
    return;

  const double simSpeed = Parameters::simulationSpeed();
  const double step = dt * simSpeed;
  simTime_ += step;

  // Sync live strategy with Parameters.
  const bool wantDijkstra = Parameters::isDijkstra();
  if (wantDijkstra != (lastStrategy_ == StrategyAlgoritm::Dijkstra)) {
    lastStrategy_ =
        wantDijkstra ? StrategyAlgoritm::Dijkstra : StrategyAlgoritm::AStar;
    setStrategyForAll(lastStrategy_);
  }

  // Build per-edge ordered lists (vehicles sorted by progress on each edge).
  Lanes lanes;
  lanes.reserve(vehicles_.size());
  for (auto &up : vehicles_) {
    Vehicle *v = up.get();
    const auto edge = v->currentEdge();
    if (edge.first < 0)
      continue; // at node or no route
    lanes[edge].emplace_back(v->edgeProgress(), v);
  }

  // Sort by progress along each edge (rear -> front).
  for (auto &[key, vec] : lanes) {
    std::sort(vec.begin(), vec.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
  }

  // Feed leader info to vehicles for IDM.
  for (auto &[key, vec] : lanes) {
    const Road *edgePtr = findEdge(graph_, key);
    if (!edgePtr) {
      continue;
    }

    for (std::size_t i = 0; i < vec.size(); ++i) {
      Vehicle *me = vec[i].second;
      me->clearLeaderInfo();

      LeaderInfo li{};
      if (i + 1 < vec.size()) {
        // Leader exists on the same edge ahead of me.
        Vehicle *lead = vec[i + 1].second;
        li.present = true;
        li.gap = std::max(0.0, lead->edgeProgress() - me->edgeProgress());
        li.leaderSpeed = lead->currentSpeed();
      } else {
        // Open road: distance to the end of the edge.
        li.present = false;
        li.gap = std::max(0.0, edgePtr->getLength() - me->edgeProgress());
        li.leaderSpeed = 0.0;
      }
      me->setLeaderInfo(li);
    }
  }

  // Advance all vehicles.
  for (auto &up : vehicles_)
    up->update(step);

  pruneArrivedVehicles();

  if (onPostUpdate_)
    onPostUpdate_(step);
}

int Simulation::spawnVehicleCar(int startId, int goalId,
                                StrategyAlgoritm algo) {
  const int id =
      spawnVehicleOf<Car>(vehicles_, graph_, congestion_, startId, goalId, algo,
                          rerouteCount_, rerouteSavedTime_);
  ensureInitialRoutes(id, startId, goalId);
  return id;
}

int Simulation::spawnVehicleTruck(int startId, int goalId,
                                  StrategyAlgoritm algo) {
  const int id =
      spawnVehicleOf<Truck>(vehicles_, graph_, congestion_, startId, goalId,
                            algo, rerouteCount_, rerouteSavedTime_);
  ensureInitialRoutes(id, startId, goalId);
  return id;
}

void Simulation::setStrategyForAll(StrategyAlgoritm algo) {
  for (auto &v : vehicles_)
    v->setStrategy(algo);
}

void Simulation::ensureInitialRoutes(int vehIdx, int startId, int goalId) {
  assert(vehIdx >= 0 && static_cast<std::size_t>(vehIdx) < vehicles_.size());
  auto &veh = vehicles_[static_cast<std::size_t>(vehIdx)];
  const auto route = veh->strategy()->computeRoute(startId, goalId, graph_);
  veh->setRoute(route);
}

std::vector<Simulation::SimSnapshotItem> Simulation::snapshot() const {
  std::vector<SimSnapshotItem> out;
  out.reserve(vehicles_.size());
  int idx = 0;
  for (const auto &ptr : vehicles_) {
    if (auto rs = ptr->renderState()) {
      out.emplace_back(SimSnapshotItem{idx, rs->fromId, rs->toId, rs->sOnEdge,
                                       rs->currentSpeed});
    }
    ++idx;
  }
  return out;
}

double Simulation::getSimTime() const noexcept { return simTime_; }

void Simulation::pruneArrivedVehicles() {
  vehicles_.erase(std::remove_if(vehicles_.begin(), vehicles_.end(),
                                 [](const std::unique_ptr<Vehicle> &v) {
                                   return v->hasArrived();
                                 }),
                  vehicles_.end());
}

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
  return sum / static_cast<double>(count);
}
