/**
 * @file RouteStrategy.h
 * @brief Strategy interface for route computation on Graph<Intersection, Road>.
 */
#ifndef ROUTE_STRATEGY_H
#define ROUTE_STRATEGY_H

#include "Easy_rider/TrafficInfrastructure/Graph.h"
#include "Easy_rider/TrafficInfrastructure/Intersection.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"

#include <vector>

class RouteStrategy {
public:
  virtual ~RouteStrategy() = default;

  /**
   * @brief Compute a route from startId to goalId.
   * @param startId Source node id.
   * @param goalId  Target node id.
   * @param graph   Graph of intersections and roads.
   * @return Sequence of node ids including startId and goalId; empty if none.
   */
  virtual std::vector<int>
  computeRoute(int startId, int goalId,
               const Graph<Intersection, Road> &graph) = 0;
};

#endif // ROUTE_STRATEGY_H
