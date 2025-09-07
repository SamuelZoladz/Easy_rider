#ifndef SIMULATION_H
#define SIMULATION_H

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/Parameters/Parameters.h"
#include "Easy_rider/RoutingStrategies/RouteStrategy.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"
#include "Easy_rider/Vehicles/Vehicle.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

/**
 * @class Simulation
 * @brief Owns the world (graph), vehicles, congestion model, and the main loop.
 */
class Simulation {
public:
  struct Stats {
    std::size_t vehicles{}; ///< Number of vehicles currently tracked.
  };

  /// @brief Build a simulation with an existing graph snapshot.
  explicit Simulation(Graph<Intersection, Road> graph)
      : graph_(std::move(graph)) {
    lastStrategy_ = Parameters::isDijkstra() ? StrategyAlgoritm::Dijkstra
                                             : StrategyAlgoritm::AStar;
  }

  ~Simulation();

  /// @brief Start advancing time in update().
  void start() {
    running_ = true;
    paused_ = false;
  }

  /// @brief Pause advancing time (state preserved).
  void pause() { paused_ = true; }

  /// @brief Stop advancing time (state preserved; call start() to resume).
  void stop() { running_ = false; }

  /// @brief Advance the simulation by dt seconds (scaled internally if needed).
  void update(double dt);

  /// @brief Create and add a new car; returns its vehicle id.
  int spawnVehicleCar(int startId, int goalId, StrategyAlgoritm algo);

  /// @brief Create and add a new truck; returns its vehicle id.
  int spawnVehicleTruck(int startId, int goalId, StrategyAlgoritm algo);

  /// @brief Replace routing strategy for all vehicles (future (re)routes).
  void setStrategyForAll(StrategyAlgoritm algo);

  [[nodiscard]] Stats stats() const { return Stats{vehicles_.size()}; }

  /// @brief Lightweight snapshot of in-flight vehicles for UI/telemetry.
  struct SimSnapshotItem {
    int id{};              ///< Vehicle id.
    int fromId{};          ///< Current edge: from intersection id.
    int toId{};            ///< Current edge: to intersection id.
    double sOnEdge{};      ///< Progress along edge [0, length].
    double currentSpeed{}; ///< Current speed in model units.
  };

  /// @return Vector of per-vehicle snapshot items.
  [[nodiscard]] std::vector<SimSnapshotItem> snapshot() const;

  [[nodiscard]] const Graph<Intersection, Road> &graph() const {
    return graph_;
  }
  [[nodiscard]] Graph<Intersection, Road> &graph() { return graph_; }

  [[nodiscard]] const CongestionModel &congestion() const {
    return congestion_;
  }
  [[nodiscard]] CongestionModel &congestion() { return congestion_; }

  [[nodiscard]] double getSimTime() const noexcept;
  [[nodiscard]] double averageSpeed() const noexcept;

  [[nodiscard]] const std::vector<std::unique_ptr<Vehicle>> &vehicles() const {
    return vehicles_;
  }

  /// @brief Register a callback invoked after each successful update(dt).
  void setOnPostUpdate(std::function<void(double)> cb) {
    onPostUpdate_ = std::move(cb);
  }

  /// @return Number of re-routes performed so far.
  [[nodiscard]] std::size_t rerouteCount() const noexcept {
    return rerouteCount_;
  }

  /// @return Cumulative time saved due to re-routing (in seconds).
  [[nodiscard]] double rerouteSavedTime() const noexcept {
    return rerouteSavedTime_;
  }

private:
  // Ensure a route exists for a newly spawned vehicle.
  void ensureInitialRoutes(int vehIdx, int startId, int goalId);

  // Remove arrived vehicles and free resources.
  void pruneArrivedVehicles();

  Graph<Intersection, Road> graph_;                ///< Road network.
  CongestionModel congestion_;                     ///< Congestion model.
  std::vector<std::unique_ptr<Vehicle>> vehicles_; ///< Owned vehicles.

  bool running_{false};
  bool paused_{false};
  double simTime_{0.0};

  std::function<void(double)> onPostUpdate_{};

  // Re-routing telemetry.
  std::size_t rerouteCount_{0};
  double rerouteSavedTime_{0.0};

  // Last chosen strategy (used for subsequent spawns if desired).
  StrategyAlgoritm lastStrategy_{StrategyAlgoritm::AStar};
};

#endif // SIMULATION_H
