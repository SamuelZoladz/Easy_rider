#ifndef SIMULATION_H
#define SIMULATION_H

#include "CongestionModel.h"
#include "Graph.h"
#include "Intersection.h"
#include "Road.h"
#include "RouteStrategy.h"
#include <memory>
#include <string>
#include <vector>

class Vehicle; // fwd
class Car;
class Truck;

/**
 * @class Simulation
 * @brief Owns the world (graph), vehicles, and time integration loop.
 */
class Simulation {
public:
  struct Stats {
    std::size_t vehicles{};
  };

  explicit Simulation(Graph<Intersection, Road> graph)
      : graph_(std::move(graph)) {
    CongestionModel::setActive(&congestion_);
  }

  void start() {
    running_ = true;
    paused_ = false;
  }
  void pause() { paused_ = true; }
  void stop() { running_ = false; }

  void setTimeScale(double s) { timeScale_ = s; }

  /// @brief Advance simulation by dt seconds (scaled by timeScale).
  void update(double dt);

  /// @brief Create and add a new vehicle; returns its id.
  int spawnVehicleCar(int startId, int goalId,
                      const std::shared_ptr<RouteStrategy> &strategy);
  int spawnVehicleTruck(int startId, int goalId,
                        const std::shared_ptr<RouteStrategy> &strategy);

  /// @brief Replace routing strategy for all vehicles.
  void setStrategyForAll(const std::shared_ptr<RouteStrategy> &strategy);

  Stats stats() const { return Stats{vehicles_.size()}; }

  const Graph<Intersection, Road> &graph() const { return graph_; }
  Graph<Intersection, Road> &graph() { return graph_; }

private:
  void ensureInitialRoutes(int vehIdx, int startId, int goalId,
                           const std::shared_ptr<RouteStrategy> &strategy);

  Graph<Intersection, Road> graph_;
  CongestionModel congestion_;
  std::vector<std::unique_ptr<Vehicle>> vehicles_;
  bool running_{false};
  bool paused_{false};
  double timeScale_{1.0};
};

#endif // SIMULATION_H