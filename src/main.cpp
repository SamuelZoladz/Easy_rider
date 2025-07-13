#include "Easy_rider/Graph.h" // your templated Graph
#include "Easy_rider/Intersection.h"
#include "Easy_rider/Road.h"
#include <iostream>

int main() {
  Intersection n1(0, 0);
  Intersection n2(3, 4);
  Intersection n3(10, 0);

  Road e1(n1, n2, 60);
  Road e2(n2, n3, 80);
  Road e3(n1, n2, 100);

  std::cout << "Road from (" << e1.getFrom().getX() << ", "
            << e1.getFrom().getY() << ") to (" << e1.getTo().getX() << ", "
            << e1.getTo().getY() << ")\n";
  std::cout << " Length: " << e1.getLength()
            << ", Max speed: " << e1.getMaxSpeed() << " km/h\n\n";

  Road defaultRoad;
  std::cout << "Default road length: " << defaultRoad.getLength()
            << ", max speed: " << defaultRoad.getMaxSpeed() << "\n\n";

  Graph<Intersection, Road> g;

  g.addNode(n1);
  g.addNode(n2);
  g.addNode(n3);

  bool added1 = g.addEdgeIfNotExists(e1);
  bool added2 = g.addEdgeIfNotExists(e2);
  bool added3 = g.addEdgeIfNotExists(e3);

  std::cout << "Added e1? " << (added1 ? "yes" : "no") << "\n";
  std::cout << "Added e2? " << (added2 ? "yes" : "no") << "\n";
  std::cout << "Added duplicate e3? " << (added3 ? "yes" : "no") << "\n\n";

  std::cout << "Graph has " << g.getNodes().size() << " nodes and "
            << g.getEdges().size() << " edges.\n\n";

  for (const auto &edge : g.getEdges()) {
    auto f = edge.getFrom().getPosition();
    auto t = edge.getTo().getPosition();
    std::cout << "  Edge: (" << f.first << "," << f.second << ") -> ("
              << t.first << "," << t.second << "), length=" << edge.getLength()
              << ", maxSpeed=" << edge.getMaxSpeed() << "\n";
  }

  return 0;
}
