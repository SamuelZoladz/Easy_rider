#include "Easy_rider/Simulation/SimulationUtils.h"
#include "Easy_rider/Vehicles/Car.h"
#include "Easy_rider/Vehicles/Truck.h"

#include <algorithm>
#include <cassert>

namespace SimulationUtils {

Graph<Intersection, Road> makeRandomRoadNetwork(const RandomNetworkParams &p,
                                                std::mt19937 &rng) {
  Graph<Intersection, Road> graph;

  std::uniform_int_distribution distX{p.minX, p.maxX};
  std::uniform_int_distribution distY{p.minY, p.maxY};

  const int kMinDist2 = p.minDistPx * p.minDistPx;
  constexpr int kMaxTriesPerNode = 2000;

  for (int i = 0; i < p.targetNodes; ++i) {
    bool placed = false;
    for (int t = 0; t < kMaxTriesPerNode && !placed; ++t) {
      int x = distX(rng), y = distY(rng);
      bool ok = true;
      for (auto const &n : graph.getNodes()) {
        auto [nx, ny] = n.getPosition();
        const int dx = x - nx, dy = y - ny;
        if (dx * dx + dy * dy < kMinDist2) {
          ok = false;
          break;
        }
      }
      if (ok) {
        graph.addNode(Intersection{x, y});
        placed = true;
      }
    }
  }

  MotorwayGenerator motorway(p.motorwayThresholdRatio, p.motorwayDefaultSpeed,
                             p.motorwayCapacity);
  HighwayGenerator highway(p.highwayDefaultSpeed, p.highwayCapacity);
  StreetGenerator streets(p.streetNumberOfNeighbors, p.streetDefaultSpeed,
                          p.streetCapacity);
  motorway.generate(graph);
  highway.generate(graph);
  streets.generate(graph);

  return graph;
}

std::vector<int> collectNodeIds(const Graph<Intersection, Road> &g) {
  std::vector<int> ids;
  ids.reserve(g.getNodes().size());
  for (auto const &n : g.getNodes())
    ids.push_back(n.getId());
  return ids;
}

FleetManager::FleetManager(Simulation &sim, const std::vector<int> &nodeIds,
                           int targetCars, int targetTrucks,
                           StrategyAlgoritm carAlg, StrategyAlgoritm truckAlg,
                           uint32_t seed)
    : sim_(sim), nodeIds_(nodeIds), targetCars_(targetCars),
      targetTrucks_(targetTrucks), carAlg_(carAlg), truckAlg_(truckAlg),
      rng_(seed) {
  assert(nodeIds_.size() >= 2 && "Needs 2 Intersection at least.");
}

int FleetManager::randomNode() {
  std::uniform_int_distribution<size_t> d(0, nodeIds_.size() - 1);
  return nodeIds_[d(rng_)];
}

std::pair<int, int> FleetManager::randomDistinctPair() {
  if (nodeIds_.size() < 2)
    return {-1, -1};
  int a = randomNode();
  int b = randomNode();
  int guard = 0;
  while (b == a && guard++ < 16)
    b = randomNode();
  if (a == b)
    return {-1, -1};
  return {a, b};
}

void FleetManager::spawnOneCar() {
  auto [s, g] = randomDistinctPair();
  if (s < 0)
    return;
  sim_.spawnVehicleCar(s, g, carAlg_);
}

void FleetManager::spawnOneTruck() {
  auto [s, g] = randomDistinctPair();
  if (s < 0)
    return;
  sim_.spawnVehicleTruck(s, g, truckAlg_);
}

void FleetManager::seedInitial() {
  for (int i = 0; i < targetCars_; ++i)
    spawnOneCar();
  for (int i = 0; i < targetTrucks_; ++i)
    spawnOneTruck();
}

void FleetManager::topUpIfNeeded() {
  int cars = 0, trucks = 0;

  const auto &vec = sim_.vehicles();
  for (const auto &up : vec) {
    if (!up)
      continue;
    if (up->renderState()) {
      if (dynamic_cast<const Truck *>(up.get()))
        ++trucks;
      else
        ++cars; // everything that isn't a Truck is a Car
    }
  }

  while (cars < targetCars_) {
    spawnOneCar();
    ++cars;
  }
  while (trucks < targetTrucks_) {
    spawnOneTruck();
    ++trucks;
  }
}

} // namespace SimulationUtils
