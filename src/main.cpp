#include "Easy_rider/Graph.h"
#include "Easy_rider/GraphVisualizer.h"
#include "Easy_rider/Intersection.h"
#include "Easy_rider/Road.h"

#include "Easy_rider/HighwayGenerator.h"
#include "Easy_rider/MotorwayGenerator.h"
#include "Easy_rider/StreetGenerator.h"

#include "Easy_rider/AStarStrategy.h"
#include "Easy_rider/DijkstraStrategy.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <unordered_map>

inline long long packEdgeKey(int u, int v) {
  return (static_cast<long long>(static_cast<unsigned int>(u)) << 32) |
         static_cast<unsigned int>(v);
}

std::unordered_map<long long, double> slowdownFactor;

struct StrictTimeContext {
  int vehicleVmax;
  const std::unordered_map<long long, double> *slowdown;
};

static StrictTimeContext g_strictTimeCtx{170, &slowdownFactor};

static double StrictTimeFnAdapter(const Road &e) {
  assert(g_strictTimeCtx.vehicleVmax > 0 && "vehicle v_max must be > 0");
  const double len = e.getLength();
  const int roadV = e.getMaxSpeed();
  assert(roadV >= 0 && "road v_max must be >= 0");

  const double baseV = std::min<double>(g_strictTimeCtx.vehicleVmax, roadV);

  double factor = 1.0;
  if (g_strictTimeCtx.slowdown) {
    const auto it =
        g_strictTimeCtx.slowdown->find(packEdgeKey(e.getFromId(), e.getToId()));
    if (it != g_strictTimeCtx.slowdown->end()) {
      factor = it->second;
      assert(factor >= 0.0 && "slowdown factor must be > = 0.0");
    }
  }

  const double effV = baseV * factor;

  if (len > 0.0) {
    assert(effV > 0.0 && "For len > 0 effective speed must be > 0");
    return len / effV;
  }
  return 0.0;
}

bool RUN_DIJKSTRA = true;

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
    for (int tries = 0; tries < kMaxTriesPerNode && !placed; ++tries) {
      int x = distX(rng);
      int y = distY(rng);

      bool ok = true;
      for (const auto &node : graph.getNodes()) {
        auto [nx, ny] = node.getPosition();
        int dx = x - nx;
        int dy = y - ny;
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
    if (!placed) {
      std::cerr << "Can't place node " << i << " after " << kMaxTriesPerNode
                << " attempts.\n";
    }
  }

  // Roads
  MotorwayGenerator farGen{0.05, 140};
  farGen.generate(graph);
  HighwayGenerator mstGen{90};
  mstGen.generate(graph);
  StreetGenerator knnGen{5, 50};
  knnGen.generate(graph);

  const auto &nodes = graph.getNodes();

  std::uniform_int_distribution<size_t> pick{0, nodes.size() - 1};
  size_t i = pick(rng), j = pick(rng);
  while (j == i)
    j = pick(rng);

  const int startId = nodes[i].getId();
  const int goalId = nodes[j].getId();

  AStarStrategy aStar{&StrictTimeFnAdapter};
  DijkstraStrategy dijkstra{&StrictTimeFnAdapter};

  auto path1 = RUN_DIJKSTRA ? dijkstra.computeRoute(startId, goalId, graph)
                            : aStar.computeRoute(startId, goalId, graph);

  std::cout << g_strictTimeCtx.vehicleVmax << " v_max path: ";
  for (auto id : path1)
    std::cout << id << ", ";

  g_strictTimeCtx.vehicleVmax = 50;

  auto path2 = RUN_DIJKSTRA ? dijkstra.computeRoute(startId, goalId, graph)
                            : aStar.computeRoute(startId, goalId, graph);

  std::cout << std::endl << g_strictTimeCtx.vehicleVmax << " v_max path: ";
  for (auto id : path2)
    std::cout << id << ", ";
  std::cout << std::endl;

  for (auto road : graph.getEdges()) {
    std::cout << "Road from " << road.getFromId() << " to " << road.getToId()
              << " with length " << road.getLength() << " and max speed "
              << road.getMaxSpeed() << std::endl;
  }

  GraphVisualizer viz{800, 600, "Random Graph SFML"};
  viz.run(graph, path1, path2);

  return 0;
}
