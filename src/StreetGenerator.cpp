#include "Easy_rider/StreetGenerator.h"

#include <algorithm>
#include <cmath>
#include <vector>

StreetGenerator::StreetGenerator(size_t k, int defaultSpeed, int capacity)
    : k_(k), defaultSpeed_(defaultSpeed), capacity_(capacity) {}

void StreetGenerator::generate(Graph<Intersection, Road> &graph) {
  auto nodes = graph.getNodes();
  size_t n = nodes.size();
  if (n < 2 || k_ <= 0)
    return;

  for (size_t i = 0; i < n; ++i) {
    std::vector<std::pair<double, size_t>> dists;
    dists.reserve(n - 1);
    for (size_t j = 0; j < n; ++j) {
      if (i == j)
        continue;
      dists.emplace_back(euclid(nodes[i], nodes[j]), j);
    }
    size_t m = std::min(k_, dists.size());
    std::nth_element(dists.begin(), dists.begin() + m, dists.end(),
                     [](auto &a, auto &b) { return a.first < b.first; });
    for (size_t t = 0; t < m; ++t) {
      const auto &A = nodes[i];
      const auto &B = nodes[dists[t].second];
      graph.addEdgeIfNotExists(Road(A, B, defaultSpeed_, capacity_));
      graph.addEdgeIfNotExists(Road(B, A, defaultSpeed_, capacity_));
    }
  }
}
