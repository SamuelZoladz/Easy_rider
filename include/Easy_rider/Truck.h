#ifndef TRUCK_H
#define TRUCK_H

#include "Vehicle.h"

/**
 * @class Truck
 * @brief Heavy truck - lower acceleration and braking.
 */
class Truck final : public Vehicle {
public:
  Truck(const Graph<Intersection, Road> &graph, CongestionModel *congestion,
        std::shared_ptr<RouteStrategy> strategy)
      : Vehicle(graph, congestion, std::move(strategy),
                IDMParams{/*a*/ 15.0, /*b*/ 20.0, /*v0*/ 25.0,
                          /*T*/ 1.8, /*s0*/ 3.0, /*delta*/ 4.0}) {}
};

#endif // TRUCK_H
