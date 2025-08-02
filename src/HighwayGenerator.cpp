#include "Easy_rider/HighwayGenerator.h"
#include "Easy_rider/Graph.h"
#include "Easy_rider/Intersection.h"
#include "Easy_rider/Road.h"

#include <algorithm>
#include <numeric>
#include <vector>

struct DisjointSet {
  std::vector<size_t> parent, rank;
  DisjointSet(size_t n) : parent(n), rank(n, 0) {
    std::iota(parent.begin(), parent.end(), 0);
  }

  size_t find(size_t x) {
    return parent[x] == x ? x : (parent[x] = find(parent[x]));
  }

  bool unite(size_t a, size_t b) {
    a = find(a);
    b = find(b);
    if (a == b)
      return false;
    if (rank[a] < rank[b])
      std::swap(a, b);
    parent[b] = a;
    if (rank[a] == rank[b])
      ++rank[a];
    return true;
  }
};

HighwayGenerator::HighwayGenerator(int defaultSpeed)
    : defaultSpeed_(defaultSpeed) {}

void HighwayGenerator::generate(Graph<Intersection, Road> &graph) {
  auto nodes = graph.getNodes();
  size_t n = nodes.size();
  if (n < 2)
    return;

  struct EdgeInfo {
    size_t u, v;
    double w;
  };
  std::vector<EdgeInfo> all;
  all.reserve(n * (n - 1) / 2);

  // collect all candidate edges
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i + 1; j < n; ++j)
      all.push_back({i, j, euclid(nodes[i], nodes[j])});

  std::sort(all.begin(), all.end(), [](auto &a, auto &b) { return a.w < b.w; });

  DisjointSet ds(n);
  using Result = Graph<Intersection, Road>::AddEdgeResult;

  // Kruskal + planar constraint
  for (auto &ei : all) {
    if (ds.find(ei.u) != ds.find(ei.v)) {
      const auto &A = nodes[ei.u];
      const auto &B = nodes[ei.v];

      auto r1 = graph.addEdgeIfNotExists(Road(A, B, defaultSpeed_));
      auto r2 = graph.addEdgeIfNotExists(Road(B, A, defaultSpeed_));

      // only unite if both inserted
      if ((r1 == Result::Success || r1 == Result::AlreadyExists) &&
          (r2 == Result::Success || r2 == Result::AlreadyExists)) {
        ds.unite(ei.u, ei.v);
      }
      // otherwise skip—later (longer) edges may connect these components
      // without crossing.
    }
  }
}
