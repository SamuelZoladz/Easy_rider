#include "Easy_rider/Parameters/Parameters.h"
#include "Easy_rider/Simulation/SimulationUtils.h"
#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"

int main() {
  std::mt19937 rng{std::random_device{}()};

  RandomNetworkParams netp;
  auto graph = SimulationUtils::makeRandomRoadNetwork(netp, rng);
  auto nodeIds = SimulationUtils::collectNodeIds(graph);

  Simulation sim(std::move(graph));

  SimulationUtils::FleetManager fleet(sim, nodeIds,
                                      /*targetCars=*/40,
                                      /*targetTrucks=*/8);

  fleet.seedInitial();
  sim.setOnPostUpdate([&](double) { fleet.topUpIfNeeded(); });

  SfmlSimulationVisualizer sfviz;
  sfviz.attachSimulation(&sim);
  sfviz.openWindow(Parameters::mainWindowWidth(),
                   Parameters::mainWindowHeight(),
                   "Random Road Network — Simulation");
  sfviz.setTimeScale(1.0);
  sfviz.run();

  return 0;
}
