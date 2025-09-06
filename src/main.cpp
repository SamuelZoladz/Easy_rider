#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/RoadGenerators/HighwayGenerator.h"
#include "Easy_rider/RoadGenerators/MotorwayGenerator.h"
#include "Easy_rider/RoadGenerators/StreetGenerator.h"
#include "Easy_rider/RoutingStrategies/AStarStrategy.h"
#include "Easy_rider/RoutingStrategies/DijkstraStrategy.h"
#include "Easy_rider/RoutingStrategies/RoutingCommon.h"
#include "Easy_rider/Simulation/Simulation.h"
#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"
#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include "Easy_rider/Visualizers/VisualizerUtils.h"

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

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

  sim.spawnVehicleCar(startId, goalId, aStar);
  sim.spawnVehicleCar(startId, goalId, aStar);
  sim.spawnVehicleCar(startId, goalId, aStar);
  sim.spawnVehicleCar(startId, goalId, aStar);
  sim.spawnVehicleTruck(startId, goalId, dijkstra);

  const uint32_t kWinW = 800, kWinH = 600;

  SfmlSimulationVisualizer sfviz;
  sfviz.attachSimulation(&sim);

  sfviz.setGraphProvider([&G]() { return viz::makeGraphDrawData(G); });
  sfviz.setVehicleProvider(
      [&sim]() { return viz::extractVehiclePositions(sim); });

  const auto drawData = viz::makeGraphDrawData(G);
  auto [minX, minY, maxX, maxY] = viz::bounds(drawData.nodePositions);
  const float worldW = std::max(1.0f, maxX - minX);
  const float worldH = std::max(1.0f, maxY - minY);
  const float padPx = 20.0f;
  // TODO To nie powinno być tak na pałe odejmowane - trzeba poprawić
  const float scaleX = (kWinW - 100.f - 2.0f * padPx) / worldW;
  const float scaleY = (kWinH - 100.f - 2.0f * padPx) / worldH;
  const float scale = std::min(scaleX, scaleY);

  viz::Vec2 originWorld{minX - padPx / scale, minY - padPx / scale};

  VisualizerView view;
  view.originX = originWorld.x;
  view.originY = originWorld.y;
  view.scale = scale;
  sfviz.setView(view);

  sfviz.setNodeRadius(4.0f);
  sfviz.setVehicleRadius(3.0f);
  sfviz.setEdgeThickness(1.5f);

  sfviz.openWindow(kWinW, kWinH, "Random Road Network — Simulation");
  sfviz.setTimeScale(1.0);
  sfviz.run(60.0);

  return 0;
}
