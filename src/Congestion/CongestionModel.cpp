/**
 * @file CongestionModel.cpp
 * @brief Implementation of tiered (halving) congestion model.
 */
#include "Easy_rider/Congestion/CongestionModel.h"

#include <cassert>
#include <cmath>

void CongestionModel::onEnterEdge(const std::pair<int, int> &edge) {
  state_[EdgeKey{edge.first, edge.second}].vehicles++;
}

void CongestionModel::onExitEdge(const std::pair<int, int> &edge) {
  const EdgeKey k{edge.first, edge.second};
  auto it = state_.find(k);
  if (it == state_.end())
    return;
  it->second.vehicles = std::max(0, it->second.vehicles - 1);
}

int CongestionModel::capacityFor(const Road &e) const {
  int cap = 0;
  cap = e.getCapacityVehicles();
  if (cap <= 0)
    cap = defaultCapacityVehicles_;
  return std::max(1, cap);
}

double CongestionModel::effectiveSpeed(const Road &e) const {
  const EdgeKey k{e.getFromId(), e.getToId()};
  double v_free = std::max(1, e.getMaxSpeed());
  if (auto it = state_.find(k); it != state_.end()) {
    if (it->second.speedLimitOverride) {
      v_free = std::min(v_free, *it->second.speedLimitOverride);
    }
  }

  // Current load N and capacity x from Road.
  const int N = [this, &k]() {
    if (auto it = state_.find(k); it != state_.end())
      return it->second.vehicles;
    return 0;
  }();
  const int x = capacityFor(e);

  if (N <= 0) {
    return v_free;
  }

  // Tier index m = ceil(N / x). Ratio = 2^-(m - 1).
  const int m = (N + x - 1) / x;                   // ceil division, m >= 1
  const int exponent = std::max(0, m - 1);         // 0 for first tier
  const double ratio = std::ldexp(1.0, -exponent); // 2^(-exponent) without pow
  const double v_eff = std::max(1e-6, v_free * ratio);
  return v_eff;
}

double CongestionModel::edgeTime(const Road &e, int vehicleMaxSpeed) const {
  const double len = std::max(1e-9, e.getLength());
  const double v_eff = effectiveSpeed(e);
  const double v_vehicle = static_cast<double>(std::max(1, vehicleMaxSpeed));
  const double v = std::min(v_vehicle, v_eff);
  return len / v;
}
