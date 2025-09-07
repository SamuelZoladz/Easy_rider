#include "Easy_rider/Parameters/Parameters.h"
#include "Easy_rider/Simulation/SimulationUtils.h"
#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"

int main() {
  std::mt19937 rng{std::random_device{}()};

  // 1) losowa sieć
  RandomNetworkParams netp;
  auto graph = SimulationUtils::makeRandomRoadNetwork(netp, rng);
  auto nodeIds = SimulationUtils::collectNodeIds(graph);

  // 2) symulacja
  Simulation sim(std::move(graph));

  // 3) menedżer floty: utrzymuj X aut i Y ciężarówek (np. 40 / 8)
  SimulationUtils::FleetManager fleet(
      sim, nodeIds,
      /*targetCars=*/40,
      /*targetTrucks=*/8, StrategyAlgoritm::AStar, StrategyAlgoritm::Dijkstra);

  // startowa obsada
  fleet.seedInitial();
  sim.setOnPostUpdate([&](double) { fleet.topUpIfNeeded(); });

  // 4) wizualizacja
  SfmlSimulationVisualizer sfviz;
  sfviz.attachSimulation(&sim);
  sfviz.openWindow(Parameters::mainWindowWidth(),
                   Parameters::mainWindowHeight(),
                   "Random Road Network — Simulation");
  sfviz.setTimeScale(1.0);
  sfviz.run();

  return 0;
}
