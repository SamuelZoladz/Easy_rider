#include "Easy_rider/RoadGenerators/MotorwayGenerator.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

struct Projection {
  double t;  // clamped to [0,1]
  double px; // projected point on segment
  double py;
};

Projection projectPointOntoSegment(const Intersection &p, const Intersection &a,
                                   const Intersection &b) {
  const auto [x0, y0] = p.getPosition();
  const auto [x1, y1] = a.getPosition();
  const auto [x2, y2] = b.getPosition();

  const double dx = x2 - x1;
  const double dy = y2 - y1;
  const double len2 = dx * dx + dy * dy;

  double t = (len2 > 0.0) ? ((x0 - x1) * dx + (y0 - y1) * dy) / len2 : 0.0;
  t = std::clamp(t, 0.0, 1.0);

  return {t, x1 + t * dx, y1 + t * dy};
}

double projectionParameter(const Intersection &p, const Intersection &a,
                           const Intersection &b) {
  return projectPointOntoSegment(p, a, b).t;
}

double pointToSeg(const Intersection &p, const Intersection &a,
                  const Intersection &b) {
  const auto pr = projectPointOntoSegment(p, a, b);
  const auto [x0, y0] = p.getPosition();
  return std::hypot(x0 - pr.px, y0 - pr.py);
}

// Ramer–Douglas–Peucker polyline simplification

using Path = std::vector<Intersection>;

void simplifyRDP(const Path &pts, double eps, Path &out) {
  if (pts.size() < 3) {
    out = pts;
    return;
  }

  // Find point farthest from the baseline (first–last).
  double maxDist = 0.0;
  std::size_t idx = 0;
  for (std::size_t i = 1; i + 1 < pts.size(); ++i) {
    const double d = pointToSeg(pts[i], pts.front(), pts.back());
    if (d > maxDist) {
      maxDist = d;
      idx = i;
    }
  }

  if (maxDist > eps) {
    // Recurse on both halves and merge (drop duplicate join point).
    Path leftPts(pts.begin(), pts.begin() + idx + 1);
    Path rightPts(pts.begin() + idx, pts.end());

    Path left, right;
    simplifyRDP(leftPts, eps, left);
    simplifyRDP(rightPts, eps, right);

    out.assign(left.begin(), left.end() - 1);
    out.insert(out.end(), right.begin(), right.end());
  } else {
    out = {pts.front(), pts.back()};
  }
}

} // namespace

MotorwayGenerator::MotorwayGenerator(double thresholdRatio, int defaultSpeed,
                                     int capacity)
    : thresholdRatio_(thresholdRatio), defaultSpeed_(defaultSpeed),
      capacity_(capacity) {}

void MotorwayGenerator::generate(Graph<Intersection, Road> &graph) {
  const auto nodes = graph.getNodes();
  const std::size_t n = nodes.size();
  if (n < 2)
    return;

  // Pick the farthest pair (A, B).
  std::size_t i0 = 0, i1 = 1;
  double maxD = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = i + 1; j < n; ++j) {
      const double d = euclid(nodes[i], nodes[j]);
      if (d > maxD) {
        maxD = d;
        i0 = i;
        i1 = j;
      }
    }
  }
  const Intersection &A = nodes[i0];
  const Intersection &B = nodes[i1];

  // Corridor width = ratio * |AB|.
  const double threshold = maxD * thresholdRatio_;
  constexpr double kSimplifyFactor = 0.5;

  // Collect nodes within corridor, ordered by projection along AB.
  std::vector<std::pair<double, std::size_t>> seq;
  seq.reserve(n);
  seq.emplace_back(0.0, i0);

  for (std::size_t k = 0; k < n; ++k) {
    if (k == i0 || k == i1)
      continue;
    if (pointToSeg(nodes[k], A, B) <= threshold) {
      const double t = projectionParameter(nodes[k], A, B);
      seq.emplace_back(t, k);
    }
  }
  seq.emplace_back(1.0, i1);

  std::sort(seq.begin(), seq.end(),
            [](const auto &a, const auto &b) { return a.first < b.first; });

  Path rawPath;
  rawPath.reserve(seq.size());
  for (const auto &[t, idx] : seq)
    rawPath.push_back(nodes[idx]);

  // Simplify with RDP.
  Path smooth;
  simplifyRDP(rawPath, threshold * kSimplifyFactor, smooth);

  // Connect consecutive intersections in the simplified corridor.
  for (std::size_t i = 0; i + 1 < smooth.size(); ++i) {
    const auto &P = smooth[i];
    const auto &Q = smooth[i + 1];
    graph.addEdgeIfNotExists(Road(P, Q, defaultSpeed_, capacity_));
    graph.addEdgeIfNotExists(Road(Q, P, defaultSpeed_, capacity_));
  }
}
