#ifndef SIMULATION_H
#define SIMULATION_H

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/RoutingStrategies/RouteStrategy.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"
#include "Easy_rider/Vehicles/Vehicle.h"

#include <memory>
#include <vector>

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
      : graph_(std::move(graph)) {}

  ~Simulation();

  void start() {
    running_ = true;
    paused_ = false;
  }
  void pause() { paused_ = true; }
  void stop() { running_ = false; }

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

  struct SimSnapshotItem {
    int id{};
    int fromId{};
    int toId{};
    double sOnEdge{};
    double currentSpeed{};
  };
  std::vector<SimSnapshotItem> snapshot() const; // <— DODANE

  const Graph<Intersection, Road> &graph() const { return graph_; }
  Graph<Intersection, Road> &graph() { return graph_; }

  const CongestionModel &congestion() const { return congestion_; }
  CongestionModel &congestion() { return congestion_; }

  double getSimTime() const noexcept;

private:
  void ensureInitialRoutes(int vehIdx, int startId, int goalId,
                           const std::shared_ptr<RouteStrategy> &strategy);

  Graph<Intersection, Road> graph_;
  CongestionModel congestion_;
  std::vector<std::unique_ptr<Vehicle>> vehicles_;
  bool running_{false};
  bool paused_{false};
  double simTime_{0.0};
};

#endif // SIMULATION_H