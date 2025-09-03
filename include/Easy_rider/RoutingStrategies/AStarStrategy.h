/**
 * @file AStarStrategy.h
 * @brief A* shortest-time strategy with injected time function and heuristic.
 */
#ifndef ASTAR_STRATEGY_H
#define ASTAR_STRATEGY_H

#include "RouteStrategy.h"
#include "RoutingCommon.h"

/**
 * @class AStarStrategy
 * @brief A* using:
 *  - g(uIdx -> vIdx)  = timeFn(edge)
 *  - h(uIdx)          = euclidean(pos[u], pos[goal]) / vmaxUpperBound
 */
class AStarStrategy final : public RouteStrategy {
public:
  explicit AStarStrategy(EdgeTimeFn timeFn) : timeFn_(timeFn) {}

  std::vector<int>
  computeRoute(int startId, int goalId,
               const Graph<Intersection, Road> &graph) override;

private:
  EdgeTimeFn timeFn_;
};

#endif // ASTAR_STRATEGY_H
