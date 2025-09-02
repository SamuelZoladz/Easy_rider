#ifndef CAR_H
#define CAR_H

#include "Vehicle.h"

/**
 * @class Car
 * @brief Passenger car - nimble, higher acceleration.
 */
class Car final : public Vehicle {
public:
  Car(const Graph<Intersection, Road> &graph, CongestionModel *congestion,
      std::shared_ptr<RouteStrategy> strategy)
      : Vehicle(graph, congestion, std::move(strategy),
                IDMParams{/*a*/ 35.0, /*b*/ 40.0, /*v0*/ 50.0,
                          /*T*/ 1.2, /*s0*/ 2.0, /*delta*/ 4.0}) {}
};

#endif // CAR_H
