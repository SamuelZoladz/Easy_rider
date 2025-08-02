#include "Easy_rider/Graph.h"
#include "Easy_rider/GraphVisualizer.h"
#include "Easy_rider/Intersection.h"
#include "Easy_rider/Road.h"

#include "Easy_rider/HighwayGenerator.h"
#include "Easy_rider/MotorwayGenerator.h"
#include "Easy_rider/StreetGenerator.h"

#include <random>

int main() {
  Graph<Intersection, Road> graph;

  std::mt19937 rng{std::random_device{}()};
  std::uniform_int_distribution<int> distX{50, 750};
  std::uniform_int_distribution<int> distY{50, 550};

  for (int i = 0; i < 30; ++i) {
    int x = distX(rng);
    int y = distY(rng);
    graph.addNode(Intersection{x, y});
  }

  MotorwayGenerator farGen{0.05, 140};
  farGen.generate(graph);

  HighwayGenerator mstGen{90};
  mstGen.generate(graph);

  StreetGenerator knnGen{5, 50};
  knnGen.generate(graph);

  GraphVisualizer viz{800, 600, "Random Graph SFML"};
  viz.run(graph);

  return 0;
}
