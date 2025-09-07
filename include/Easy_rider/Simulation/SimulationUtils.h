#ifndef SIMULATIONUTILS_H
#define SIMULATIONUTILS_H

#include "Easy_rider/RoadGenerators/HighwayGenerator.h"
#include "Easy_rider/RoadGenerators/MotorwayGenerator.h"
#include "Easy_rider/RoadGenerators/StreetGenerator.h"
#include "Easy_rider/Simulation/Simulation.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"

#include <random>
#include <vector>

#include "Easy_rider/Parameters/Parameters.h"

struct RandomNetworkParams {
  int targetNodes = Parameters::targetNodes();
  int minDistPx = Parameters::minDistPx();

  int minX = Parameters::networkMinX();
  int maxX = Parameters::networkMaxX();
  int minY = Parameters::networkMinY();
  int maxY = Parameters::networkMaxY();

  double motorwayThresholdRatio = Parameters::motorwayThresholdRatio();
  int motorwayDefaultSpeed = Parameters::motorwayDefaultSpeed();
  int motorwayCapacity = Parameters::motorwayCapacity();

  int highwayDefaultSpeed = Parameters::highwayDefaultSpeed();
  int highwayCapacity = Parameters::highwayCapacity();

  int streetNumberOfNeighbors = Parameters::streetNumberOfNeighbors();
  int streetDefaultSpeed = Parameters::streetDefaultSpeed();
  int streetCapacity = Parameters::streetCapacity();
};

namespace SimulationUtils {

Graph<Intersection, Road> makeRandomRoadNetwork(const RandomNetworkParams &p,
                                                std::mt19937 &rng);

std::vector<int> collectNodeIds(const Graph<Intersection, Road> &g);

class FleetManager {
public:
  FleetManager(Simulation &sim, const std::vector<int> &nodeIds, int targetCars,
               int targetTrucks,
               StrategyAlgoritm carAlg = StrategyAlgoritm::AStar,
               StrategyAlgoritm truckAlg = StrategyAlgoritm::Dijkstra,
               uint32_t seed = std::random_device{}());

  void seedInitial();
  void topUpIfNeeded();

private:
  Simulation &sim_;
  std::vector<int> nodeIds_;
  int targetCars_;
  int targetTrucks_;
  StrategyAlgoritm carAlg_;
  StrategyAlgoritm truckAlg_;
  std::mt19937 rng_;

  int randomNode();
  std::pair<int, int> randomDistinctPair();

  void spawnOneCar();
  void spawnOneTruck();
};

} // namespace SimulationUtils

#endif // SIMULATIONUTILS_H
