#include "Easy_rider/CongestionModel.h"
#include <cassert>

void CongestionModel::onEnterEdge(const std::pair<int, int> &edge) {
  state_[EdgeKey{edge.first, edge.second}].vehicles++;
}

void CongestionModel::onExitEdge(const std::pair<int, int> &edge) {
  const EdgeKey k{edge.first, edge.second};
  auto it = state_.find(k);
  if (it == state_.end())
    return;

  it->second.vehicles = std::max(0, it->second.vehicles - 1);

  if (it->second.vehicles == 0 && !it->second.speedLimitOverride) {
    state_.erase(it);
  }
}

int CongestionModel::vehiclesOnEdge(const EdgeKey &k) const {
  auto it = state_.find(k);
  if (it == state_.end())
    return 0;
  return it->second.vehicles;
}

void CongestionModel::setEdgeSpeedLimit(const EdgeKey &k, double limit) {
  state_[k].speedLimitOverride = std::max(0.0, limit);
}

void CongestionModel::clearEdgeSpeedLimit(const EdgeKey &k) {
  auto it = state_.find(k);
  if (it == state_.end())
    return;
  it->second.speedLimitOverride.reset();

  if (it->second.vehicles == 0) {
    state_.erase(it);
  }
}

double CongestionModel::effectiveSpeed(const Road &e) const {
  const double base = static_cast<double>(e.getMaxSpeed());
  const EdgeKey k{e.getFromId(), e.getToId()};
  const int load = vehiclesOnEdge(k);

  double limit = base;
  if (auto it = state_.find(k); it != state_.end()) {
    if (it->second.speedLimitOverride) {
      limit = std::min(limit, *it->second.speedLimitOverride);
    }
  }

  double factor = 1.0;
  if (load <= 1) {
    factor = 1.0;
  } else if (load < threshold_) {
    factor = 0.75;
  } else if (load <= 2 * threshold_) {
    factor = 0.5;
  } else {
    factor = 0.25;
  }

  return std::max(0.001, limit * factor);
}

double CongestionModel::edgeTime(const Road &e, int vehicleMaxSpeed) const {
  const double len = static_cast<double>(e.getLength());
  const double v_eff = effectiveSpeed(e);
  const double v_vehicle = static_cast<double>(std::max(1, vehicleMaxSpeed));
  const double v = std::min(v_vehicle, v_eff);
  return len / v;
}
