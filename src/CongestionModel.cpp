#include "Easy_rider/CongestionModel.h"

#include <algorithm>
#include <cassert>

CongestionModel *CongestionModel::s_active_ = nullptr;

void CongestionModel::onEnterEdge(const std::pair<int, int> &edge) {
  ++load_[EdgeKey{edge.first, edge.second}];
}

void CongestionModel::onExitEdge(const std::pair<int, int> &edge) {
  auto k = EdgeKey{edge.first, edge.second};
  auto it = load_.find(k);
  if (it != load_.end()) {
    if (--it->second <= 0)
      load_.erase(it);
  }
}

int CongestionModel::vehiclesOnEdge(const EdgeKey &k) const {
  auto it = load_.find(k);
  return it == load_.end() ? 0 : it->second;
}

bool CongestionModel::isCongested(const Road &e) const {
  EdgeKey k{e.getFromId(), e.getToId()};
  return vehiclesOnEdge(k) >= threshold_;
}

void CongestionModel::setEdgeSpeedLimit(const EdgeKey &k, double limit) {
  manualSpeedLimit_[k] = std::max(0.0, limit);
}

void CongestionModel::clearEdgeSpeedLimit(const EdgeKey &k) {
  manualSpeedLimit_.erase(k);
}

double CongestionModel::effectiveSpeed(const Road &e) const {
  const double base = static_cast<double>(e.getMaxSpeed());
  EdgeKey k{e.getFromId(), e.getToId()};
  const int load = vehiclesOnEdge(k);

  double limit = base;
  if (auto it = manualSpeedLimit_.find(k); it != manualSpeedLimit_.end())
    limit = std::min(limit, it->second);

  double factor = 1.0;
  if (load <= 1)
    factor = 1.0;
  else if (load <= 3)
    factor = 0.8;
  else if (load <= 6)
    factor = 0.5;
  else
    factor = 0.25;

  double v = std::max(0.001, std::min(limit, base) * factor);
  return v; // units/s
}

double CongestionModel::edgeTime(const Road &e) const {
  const double len = e.getLength();
  const double v = effectiveSpeed(e);
  return len / v; // seconds
}

double CongestionModel::StaticEdgeTime(const Road &e) {
  assert(s_active_ && "No active CongestionModel installed");
  return s_active_->edgeTime(e);
}
