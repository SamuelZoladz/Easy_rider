/**
 * @file RoutingCommon.h
 * @brief Shared types and helpers for shortest-time routing strategies.
 */
#ifndef ROUTING_COMMON_H
#define ROUTING_COMMON_H

#include "Graph.h"
#include "Intersection.h"
#include "Road.h"
#include <cassert>
#include <cmath>
#include <functional>
#include <vector>

/**
 * @brief External provider of travel time for a directed edge (Road).
 * @details
 * MUST return a finite, non-negative time in the same units across the graph.
 */
using EdgeTimeFn = std::function<double(const Road &)>;

/**
 * @brief Rebuild a path of node ids from parent indices.
 * @param startIdx Index of the start node.
 * @param goalIdx  Index of the goal node.
 * @param parent   Parent array of indices (-1 for root/unset).
 * @param graph    Graph to map indices -> ids.
 * @return Sequence of node ids startId ... goalId or empty if unreachable.
 */
inline std::vector<int>
rebuildPathIdsFromParents(int startIdx, int goalIdx,
                          const std::vector<int> &parent,
                          const Graph<Intersection, Road> &graph) {
  std::vector<int> ids;
  if (startIdx == goalIdx) {
    ids.push_back(graph.getNodes().at(static_cast<size_t>(startIdx)).getId());
    return ids;
  }
  if (goalIdx < 0 || goalIdx >= static_cast<int>(parent.size()))
    return ids;
  if (parent[goalIdx] == -1)
    return ids;

  int cur = goalIdx;
  while (cur != -1) {
    ids.push_back(graph.getNodes().at(static_cast<size_t>(cur)).getId());
    cur = parent[cur];
  }
  std::ranges::reverse(ids);
  if (!ids.empty()) {
    const int firstIdx = static_cast<int>(graph.indexOfId(ids.front()));
    if (firstIdx != startIdx)
      ids.clear();
  }
  return ids;
}

/**
 * @brief Compute an optimistic upper bound on effective speed for A* heuristic.
 * @details
 * The bound is max over edges of (edge.length / timeFn(edge)).
 * Asserts that the result is positive and finite.
 */
inline double computeVmaxUpperBound(const Graph<Intersection, Road> &graph,
                                    EdgeTimeFn timeFn) {
  assert(timeFn && "timeFn must not be null");
  double vmax = 0.0;
  for (const auto &e : graph.getEdges()) {
    const double len = e.getLength();
    const double t = timeFn(e);
    assert(std::isfinite(t) && t >= 0.0 &&
           "timeFn(edge) must be finite and >= 0");
    if (len > 0.0) {
      assert(t > 0.0 &&
             "Positive-length edge must have strictly positive time");
      const double s = len / t;
      if (s > vmax)
        vmax = s;
    }
  }
  assert(vmax > 0.0 &&
         "No positive effective speed found. Check timeFn and edge lengths.");
  return vmax;
}

#endif // ROUTING_COMMON_H
