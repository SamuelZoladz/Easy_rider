#include "Easy_rider/Simulation.h"

#include "Easy_rider/AStarStrategy.h"
#include "Easy_rider/Car.h"
#include "Easy_rider/DijkstraStrategy.h"
#include "Easy_rider/Truck.h"
#include "Easy_rider/Vehicle.h"

void Simulation::update(double dt) {
  if (!running_ || paused_)
    return;
  const double sdt = dt * timeScale_;
  for (auto &v : vehicles_)
    v->update(sdt);
}

void Simulation::ensureInitialRoutes(
    int vehIdx, int startId, int goalId,
    const std::shared_ptr<RouteStrategy> &strategy) {
  auto route = strategy->computeRoute(startId, goalId, graph_);
  vehicles_[vehIdx]->setRoute(route);
}

int Simulation::spawnVehicleCar(
    int startId, int goalId, const std::shared_ptr<RouteStrategy> &strategy) {
  vehicles_.push_back(std::make_unique<Car>(graph_, &congestion_, strategy));
  int idx = static_cast<int>(vehicles_.size() - 1);
  ensureInitialRoutes(idx, startId, goalId, strategy);
  return vehicles_[idx]->id();
}

int Simulation::spawnVehicleTruck(
    int startId, int goalId, const std::shared_ptr<RouteStrategy> &strategy) {
  vehicles_.push_back(std::make_unique<Truck>(graph_, &congestion_, strategy));
  int idx = static_cast<int>(vehicles_.size() - 1);
  ensureInitialRoutes(idx, startId, goalId, strategy);
  return vehicles_[idx]->id();
}

void Simulation::setStrategyForAll(
    const std::shared_ptr<RouteStrategy> &strategy) {
  for (auto &v : vehicles_)
    v->setStrategy(strategy);
}