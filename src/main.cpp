#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "Easy_rider/Graph.h"
#include "Easy_rider/Intersection.h"
#include "Easy_rider/Road.h"

#include "Easy_rider/HighwayGenerator.h"
#include "Easy_rider/MotorwayGenerator.h"
#include "Easy_rider/StreetGenerator.h"

#include "Easy_rider/AStarStrategy.h"
#include "Easy_rider/CongestionModel.h"
#include "Easy_rider/DijkstraStrategy.h"
#include "Easy_rider/RoutingCommon.h"
#include "Easy_rider/Simulation.h"

#include "Easy_rider/GraphVisualizer.h"

int main() {
  Graph<Intersection, Road> graph;

  std::mt19937 rng{std::random_device{}()};
  std::uniform_int_distribution<int> distX{50, 750};
  std::uniform_int_distribution<int> distY{50, 550};

  constexpr int kTargetCount = 30;
  constexpr int kMinDist = 30;
  constexpr int kMinDist2 = kMinDist * kMinDist;
  constexpr int kMaxTriesPerNode = 2000;

  for (int i = 0; i < kTargetCount; ++i) {
    bool placed = false;
    for (int t = 0; t < kMaxTriesPerNode && !placed; ++t) {
      int x = distX(rng), y = distY(rng);
      bool ok = true;
      for (auto const &n : graph.getNodes()) {
        auto [nx, ny] = n.getPosition();
        int dx = x - nx, dy = y - ny;
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

  MotorwayGenerator motorway(0.07, 39, 4);
  HighwayGenerator highway(25, 2);
  StreetGenerator streets(3, 14, 1);
  motorway.generate(graph);
  highway.generate(graph);
  streets.generate(graph);

  const int startId = graph.getNodes().front().getId();
  const int goalId = graph.getNodes().back().getId();

  Simulation sim(std::move(graph));
  auto &G = sim.graph();

  EdgeTimeFn carTimeFn = [&sim](const Road &e) {
    return sim.congestion().edgeTime(e, /*car vmax*/ 50);
  };
  EdgeTimeFn truckTimeFn = [&sim](const Road &e) {
    return sim.congestion().edgeTime(e, /*truck vmax*/ 25);
  };

  auto aStar = std::make_shared<AStarStrategy>(truckTimeFn);
  auto dijkstra = std::make_shared<DijkstraStrategy>(carTimeFn);

  auto carPath = aStar->computeRoute(startId, goalId, G);
  auto truckPath = dijkstra->computeRoute(startId, goalId, G);

  int carId = sim.spawnVehicleCar(startId, goalId, dijkstra);
  sim.spawnVehicleCar(startId, goalId, dijkstra);
  sim.spawnVehicleCar(startId, goalId, dijkstra);
  sim.spawnVehicleCar(startId, goalId, dijkstra);
  int truckId = sim.spawnVehicleTruck(startId, goalId, aStar);
  sim.start();

  GraphVisualizer viz{800, 600,
                      "Random Road Network — Car & Truck (Simulation)"};
  viz.runWithSimulation(sim, carPath, truckPath, carId, truckId);
  return 0;
}
