#include "Easy_rider/Simulation.h"
#include "Easy_rider/Car.h"
#include "Easy_rider/RouteStrategy.h"
#include "Easy_rider/Truck.h"
#include "Easy_rider/Vehicle.h"

#include <cassert>
#include <optional>
#include <utility>

Simulation::~Simulation() =
    default; // kompletna definicja przy pełnym typie Vehicle

void Simulation::update(double dt) {
  if (!running_ || paused_)
    return;
  const double step = dt * timeScale_;
  for (auto &v : vehicles_)
    v->update(step);
}

int Simulation::spawnVehicleCar(
    int startId, int goalId, const std::shared_ptr<RouteStrategy> &strategy) {
  auto veh = std::make_unique<Car>(graph_, &congestion_, strategy);
  int id = static_cast<int>(vehicles_.size());
  vehicles_.push_back(std::move(veh));
  ensureInitialRoutes(id, startId, goalId, strategy);
  return id;
}

int Simulation::spawnVehicleTruck(
    int startId, int goalId, const std::shared_ptr<RouteStrategy> &strategy) {
  auto veh = std::make_unique<Truck>(graph_, &congestion_, strategy);
  int id = static_cast<int>(vehicles_.size());
  vehicles_.push_back(std::move(veh));
  ensureInitialRoutes(id, startId, goalId, strategy);
  return id;
}

void Simulation::setStrategyForAll(
    const std::shared_ptr<RouteStrategy> &strategy) {
  for (auto &v : vehicles_)
    v->setStrategy(strategy);
}

void Simulation::ensureInitialRoutes(
    int vehIdx, int startId, int goalId,
    const std::shared_ptr<RouteStrategy> &strategy) {
  assert(vehIdx >= 0 && static_cast<std::size_t>(vehIdx) < vehicles_.size());
  auto route = strategy->computeRoute(startId, goalId, graph_);
  vehicles_[static_cast<std::size_t>(vehIdx)]->setRoute(route);
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
