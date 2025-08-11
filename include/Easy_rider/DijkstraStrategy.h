/**
 * @file DijkstraStrategy.h
 * @brief Dijkstra shortest-time strategy with injected time function.
 */
#ifndef DIJKSTRA_STRATEGY_H
#define DIJKSTRA_STRATEGY_H

#include "RouteStrategy.h"
#include "RoutingCommon.h"

/**
 * @class DijkstraStrategy
 * @brief Dijkstra using a min-heap; edge weight is always travel time.
 *
 * @details
 * Edge time is provided by an external function:
 *   w(uIdx -> vIdx) = timeFn(edge)
 * The strategy works on node indices and returns node ids at the end.
 */
class DijkstraStrategy final : public RouteStrategy {
public:
  explicit DijkstraStrategy(EdgeTimeFn timeFn) : timeFn_(timeFn) {}

  std::vector<int>
  computeRoute(int startId, int goalId,
               const Graph<Intersection, Road> &graph) override;

private:
  EdgeTimeFn timeFn_;
};

#endif // DIJKSTRA_STRATEGY_H
