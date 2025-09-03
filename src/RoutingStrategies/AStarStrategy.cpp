#include "Easy_rider/RoutingStrategies/AStarStrategy.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <queue>

std::vector<int>
AStarStrategy::computeRoute(int startId, int goalId,
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
  const double vmax = computeVmaxUpperBound(graph, timeFn_);

  auto euclidIdx = [&](int aIdx, int bIdx) -> double {
    const auto &a = nodes.at(static_cast<size_t>(aIdx));
    const auto &b = nodes.at(static_cast<size_t>(bIdx));
    const double dx = static_cast<double>(a.getPosition().first) -
                      static_cast<double>(b.getPosition().first);
    const double dy = static_cast<double>(a.getPosition().second) -
                      static_cast<double>(b.getPosition().second);
    return std::hypot(dx, dy);
  };

  auto h = [&](int uIdx) -> double { return euclidIdx(uIdx, gIdx) / vmax; };

  std::vector<double> gScore(n, INF);
  std::vector<int> parent(n, -1);
  std::vector<char> closed(n, 0);

  using QElem = std::pair<double, int>; // (fScore, idx)
  std::priority_queue<QElem, std::vector<QElem>, std::greater<>> open;

  gScore[sIdx] = 0.0;
  open.emplace(h(sIdx), sIdx);

  while (!open.empty()) {
    auto [f, uIdx] = open.top();
    open.pop();
    if (closed[uIdx])
      continue;
    closed[uIdx] = 1;
    if (uIdx == gIdx)
      break;

    for (const auto &nbr : graph.outgoing(uIdx)) {
      const int vIdx = nbr.first;
      const Road &e = nbr.second;

      const double w = timeFn_(e);
      assert(std::isfinite(w) && w >= 0.0 &&
             "timeFn(edge) must be finite and >= 0");

      const double tentative = gScore[uIdx] + w;
      if (tentative < gScore[vIdx]) {
        gScore[vIdx] = tentative;
        parent[vIdx] = uIdx;
        const double fScore = tentative + h(vIdx);
        open.emplace(fScore, vIdx);
      }
    }
  }

  return rebuildPathIdsFromParents(sIdx, gIdx, parent, graph);
}
