#ifndef TRUCK_H
#define TRUCK_H

#include "Vehicle.h"

/**
 * @class Truck
 * @brief Heavy truck — lower acceleration and braking.
 */
class Truck final : public Vehicle {
public:
  Truck(const Graph<Intersection, Road> &graph, CongestionModel *congestion,
        std::shared_ptr<RouteStrategy> strategy)
      : Vehicle(graph, congestion, std::move(strategy),
                /*maxSpeed*/ 25.0, /*acc*/ 15, /*brake*/ 20) {}
};

#endif // TRUCK_H
