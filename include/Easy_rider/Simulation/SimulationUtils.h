#ifndef SIMULATIONUTILS_H
#define SIMULATIONUTILS_H

#include "Easy_rider/RoadGenerators/HighwayGenerator.h"
#include "Easy_rider/RoadGenerators/MotorwayGenerator.h"
#include "Easy_rider/RoadGenerators/StreetGenerator.h"
#include "Easy_rider/Simulation/Simulation.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"

#include <cstddef>
#include <random>
#include <utility>
#include <vector>

#include "Easy_rider/Parameters/Parameters.h"

/**
 * @brief Tunables for building a random road network.
 *
 * Defaults are sourced from Parameters to keep config in one place.
 */
struct RandomNetworkParams {
  // Node count & spacing
  int targetNodes = Parameters::targetNodes();
  int minDistPx = Parameters::minDistPx();

  // Bounding box (screen/model coordinates)
  int minX = Parameters::networkMinX();
  int maxX = Parameters::networkMaxX();
  int minY = Parameters::networkMinY();
  int maxY = Parameters::networkMaxY();

  // Motorway corridor construction
  double motorwayThresholdRatio = Parameters::motorwayThresholdRatio();
  int motorwayDefaultSpeed = Parameters::motorwayDefaultSpeed();
  int motorwayCapacity = Parameters::motorwayCapacity();

  // Highways
  int highwayDefaultSpeed = Parameters::highwayDefaultSpeed();
  int highwayCapacity = Parameters::highwayCapacity();

  // Streets
  int streetNumberOfNeighbors = Parameters::streetNumberOfNeighbors();
  int streetDefaultSpeed = Parameters::streetDefaultSpeed();
  int streetCapacity = Parameters::streetCapacity();
};

namespace SimulationUtils {

/**
 * @brief Create a random network using Motorway/Highway/Street generators.
 * @param p    Parameters controlling size, speeds, and capacities.
 * @param rng  RNG used for sampling node positions and connections.
 * @return Graph of intersections and roads.
 */
[[nodiscard]] Graph<Intersection, Road>
makeRandomRoadNetwork(const RandomNetworkParams &p, std::mt19937 &rng);

/**
 * @brief Collect all node ids from the graph in a flat vector.
 */
[[nodiscard]] std::vector<int>
collectNodeIds(const Graph<Intersection, Road> &g);

/**
 * @class FleetManager
 * @brief Maintains a target number of cars/trucks in the simulation.
 *
 * Typical usage:
 *  - Call seedInitial() once after creating the Simulation.
 *  - Call topUpIfNeeded() periodically to keep the fleet at target levels.
 */
class FleetManager {
public:
  /**
   * @param sim          Simulation to spawn vehicles into (non-owning).
   * @param nodeIds      Candidate node ids to use as (start, goal) endpoints.
   * @param targetCars   Desired number of cars present.
   * @param targetTrucks Desired number of trucks present.
   * @param seed         RNG seed (defaults to random_device()).
   */
  explicit FleetManager(Simulation &sim, const std::vector<int> &nodeIds,
                        int targetCars, int targetTrucks,
                        uint32_t seed = std::random_device{}());

  /// @brief Spawn up to target levels once.
  void seedInitial();

  /// @brief Ensure current counts stay at or near targets (top-up only).
  void topUpIfNeeded();

private:
  Simulation &sim_;
  const std::vector<int> nodeIds_;
  int targetCars_;
  int targetTrucks_;
  StrategyAlgoritm alg_;
  std::mt19937 rng_;

  /// @return Random node id from nodeIds_.
  [[nodiscard]] int randomNode();

  /// @return Distinct (start, goal) node ids.
  [[nodiscard]] std::pair<int, int> randomDistinctPair();

  void spawnOneCar();
  void spawnOneTruck();
};

} // namespace SimulationUtils

#endif // SIMULATIONUTILS_H
