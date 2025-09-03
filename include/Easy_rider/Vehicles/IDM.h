#ifndef IDM_H
#define IDM_H

#include <algorithm>
#include <cmath>

/**
 * @file IDM.h
 * @brief Intelligent Driver Model (IDM) utilities.
 *
 * This header defines a small, header-only helper for computing longitudinal
 * acceleration according to the Intelligent Driver Model. It is designed to be
 * embedded into Vehicle.
 */

/**
 * @struct IDMParams
 * @brief Tunable parameters for IDM.
 *
 * Units:
 *  - v0:       desired speed [units/s]
 *  - a:        maximum acceleration [units/s^2]
 *  - b:        comfortable braking (positive) [units/s^2]
 *  - T:        desired time headway [s]
 *  - s0:       minimum distance at standstill [units]
 *  - delta:    acceleration exponent [-] (usually 4)
 */
struct IDMParams {
  double v0{13.9};   ///< Desired speed (fallback). Will be clamped per-edge.
  double a{1.2};     ///< Max acceleration.
  double b{2.0};     ///< Comfortable braking (positive).
  double T{1.4};     ///< Desired time headway.
  double s0{2.0};    ///< Minimum jam distance.
  double delta{4.0}; ///< Acceleration exponent.
};

/**
 * @struct LeaderInfo
 * @brief Information about the nearest leader (vehicle ahead) on the same edge.
 *
 * All distances are measured along the edge.
 */
struct LeaderInfo {
  bool present{false};     ///< Whether there is a leader on the same edge.
  double gap{1e9};         ///< Free distance to the leader's rear bumper.
  double leaderSpeed{0.0}; ///< Leader speed.
};

namespace idm_detail {
/**
 * @brief Compute the desired dynamic distance s* for IDM.
 * @param v follower speed
 * @param dv closing speed = v - v_leader (positive when approaching)
 * @param p IDM parameters
 */
inline double desired_gap(double v, double dv, const IDMParams &p) {
  const double denom = 2.0 * std::sqrt(std::max(1e-9, p.a * p.b));
  const double sStar = p.s0 + std::max(0.0, v * p.T + (v * dv) / denom);
  return sStar;
}
} // namespace idm_detail

/**
 * @brief Compute IDM acceleration.
 *
 * @param v follower current speed
 * @param v0 desired speed limit for the current context
 * @param gap free distance ahead
 * @param dv closing speed = v - v_leader
 * @param p IDM parameters
 * @return longitudinal acceleration
 */
inline double idm_accel(double v, double v0, double gap, double dv,
                        const IDMParams &p) {
  const double vv = std::max(0.0, v);
  const double v0c = std::max(1e-3, v0);
  const double termFree = std::pow(vv / v0c, p.delta);
  const double sStar = idm_detail::desired_gap(vv, dv, p);
  const double sRatio = sStar / std::max(1e-3, gap);
  const double acc = p.a * (1.0 - termFree - sRatio * sRatio);
  return acc;
}

#endif // IDM_H
