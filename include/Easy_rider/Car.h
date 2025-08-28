#ifndef CAR_H
#define CAR_H

#include "Vehicle.h"

/**
 * @class Car
 * @brief Passenger car — nimble, higher acceleration.
 */
class Car final : public Vehicle {
public:
  Car(const Graph<Intersection, Road> &graph, CongestionModel *congestion,
      std::shared_ptr<RouteStrategy> strategy)
      : Vehicle(graph, congestion, std::move(strategy),
                /*maxSpeed*/ 50.0, /*acc*/ 35, /*brake*/ 40) {}
};

#endif // CAR_H
