#ifndef TRAFFIC_EVENTS_H
#define TRAFFIC_EVENTS_H

#include "CongestionModel.h"

/**
 * @class TrafficEvents
 * @brief High-level helpers to inject incidents/blocks into the model.
 */
class TrafficEvents {
public:
  explicit TrafficEvents(CongestionModel &m) : model_(m) {}

  /// @brief Block an edge by capping its speed to near-zero.
  void blockEdge(int fromId, int toId) {
    model_.setEdgeSpeedLimit(EdgeKey{fromId, toId}, 0.01);
  }

  /// @brief Clear a block/incident for an edge.
  void clearEdge(int fromId, int toId) {
    model_.clearEdgeSpeedLimit(EdgeKey{fromId, toId});
  }

private:
  CongestionModel &model_;
};

#endif // TRAFFIC_EVENTS_H