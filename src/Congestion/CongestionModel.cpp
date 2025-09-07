/**
 * @file CongestionModel.cpp
 * @brief Implementation of tiered (halving) congestion model.
 */

#include "Easy_rider/Congestion/CongestionModel.h"
#include "Easy_rider/TrafficInfrastructure/Road.h"

#include <algorithm>
#include <cmath>

void CongestionModel::onEnterEdge(const EdgeKey &edge) {
  state_[edge].vehicles++;
}

void CongestionModel::onExitEdge(const EdgeKey &edge) {
  auto it = state_.find(edge);
  if (it == state_.end())
    return;

  it->second.vehicles = std::max(0, it->second.vehicles - 1);
}

int CongestionModel::capacityFor(const Road &road) const {
  int cap = road.getCapacityVehicles();
  if (cap <= 0)
    cap = defaultCapacityVehicles_;
  return std::max(1, cap);
}

double CongestionModel::effectiveSpeed(const Road &road) const {
  const EdgeKey key{road.getFromId(), road.getToId()};

  double v_free = std::max(1, road.getMaxSpeed());

  int N = 0; // current load on the edge
  if (auto it = state_.find(key); it != state_.end()) {
    N = it->second.vehicles;
    if (it->second.speedLimitOverride) {
      v_free = std::min(v_free, *it->second.speedLimitOverride);
    }
  }

  if (N <= 0)
    return v_free;

  const int x = capacityFor(road);

  // Tier index m = ceil(N/x). Divisor = 2^(m-1).
  const int m = (N + x - 1) / x;           // ceil division, m >= 1
  const int exponent = std::max(0, m - 1); // 0 in first tier
  const double ratio = std::ldexp(1.0, -exponent);
  const double v_eff = std::max(1e-6, v_free * ratio);
  return v_eff;
}

double CongestionModel::edgeTime(const Road &road, int vehicleMaxSpeed) const {
  // Time = length / min(vehicleMaxSpeed, effectiveSpeed(road)).
  const double len = std::max(1e-9, road.getLength());
  const double v_eff = effectiveSpeed(road);
  const double v_vehicle = static_cast<double>(std::max(1, vehicleMaxSpeed));
  const double v = std::min(v_vehicle, v_eff);
  return len / v;
}
