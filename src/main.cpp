#include "Easy_rider/Node.h"
#include <iostream>

int main() {
  Node n1;
  n1.setPosition(3, 2);
  auto [x1, y1] = n1.getPosition();
  std::cout << "n1 position: (" << x1 << ", " << y1 << ")\n";

  const Node n2(10, 20);
  std::cout << "n2 position: (" << n2.getX() << ", " << n2.getY() << ")\n";

  return 0;
}
