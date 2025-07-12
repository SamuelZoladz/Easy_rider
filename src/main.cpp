#include "Easy_rider/Edge.h"
#include "Easy_rider/Node.h"
#include <iostream>

int main() {
  Node n1(0, 0);
  Node n2(3, 4);

  Edge e(n1, n2, 60);

  std::cout << "Edge from (" << e.getFrom().getX() << ", " << e.getFrom().getY()
            << ") to (" << e.getTo().getX() << ", " << e.getTo().getY()
            << ")\n";
  std::cout << "Length: " << e.getLength() << "\n";
  std::cout << "Max speed: " << e.getMaxSpeed() << " km/h\n";

  Edge defaultEdge;
  std::cout << "\nDefault edge length: " << defaultEdge.getLength()
            << ", max speed: " << defaultEdge.getMaxSpeed() << "\n";

  return 0;
}
