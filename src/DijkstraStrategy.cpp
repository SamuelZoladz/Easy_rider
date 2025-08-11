#include "Easy_rider/DijkstraStrategy.h"
#include <cassert>
#include <functional>
#include <limits>
#include <queue>

std::vector<int>
DijkstraStrategy::computeRoute(int startId, int goalId,
                               const Graph<Intersection, Road> &graph) {
  assert(timeFn_ && "timeFn must not be null");

  const auto &nodes = graph.getNodes();
  if (nodes.empty())
    return {};

  int sIdx, gIdx;
  try {
    sIdx = static_cast<int>(graph.indexOfId(startId));
    gIdx = static_cast<int>(graph.indexOfId(goalId));
  } catch (...) {
    return {};
  }

  const int n = static_cast<int>(nodes.size());
  const double INF = std::numeric_limits<double>::infinity();

  std::vector<double> dist(n, INF);
  std::vector<int> parent(n, -1);
  std::vector<char> used(n, 0);

  using QElem = std::pair<double, int>; // (dist, idx)
  std::priority_queue<QElem, std::vector<QElem>, std::greater<>> pq;

  dist[sIdx] = 0.0;
  pq.emplace(0.0, sIdx);

  while (!pq.empty()) {
    auto [du, uIdx] = pq.top();
    pq.pop();
    if (used[uIdx])
      continue;
    used[uIdx] = 1;
    if (uIdx == gIdx)
      break;

    for (const auto &nbr : graph.outgoing(uIdx)) {
      const int vIdx = nbr.first;
      const Road &e = nbr.second;

      const double w = timeFn_(e);
      assert(std::isfinite(w) && w >= 0.0 &&
             "timeFn(edge) must be finite and >= 0");

      const double nd = dist[uIdx] + w;
      if (nd < dist[vIdx]) {
        dist[vIdx] = nd;
        parent[vIdx] = uIdx;
        pq.emplace(nd, vIdx);
      }
    }
  }

  return rebuildPathIdsFromParents(sIdx, gIdx, parent, graph);
}
