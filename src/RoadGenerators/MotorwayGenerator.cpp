#include "Easy_rider/RoadGenerators/MotorwayGenerator.h"

#include <algorithm>
#include <cmath>
#include <vector>

// TODO: I don't like this implementation of the algorithm - it's too
// complicated and still
//  doesn't create “motorways” - it needs to be changed in the future.
//--------------------------------------------------------------------------------
//  helpers: distance, projection, point->segment
//--------------------------------------------------------------------------------

static double projectionParameter(const Intersection &p, const Intersection &a,
                                  const Intersection &b) {
  auto [x0, y0] = p.getPosition();
  auto [x1, y1] = a.getPosition();
  auto [x2, y2] = b.getPosition();
  double dx = x2 - x1, dy = y2 - y1;
  double len2 = dx * dx + dy * dy;
  if (len2 == 0.0)
    return 0.0;
  double t = ((x0 - x1) * dx + (y0 - y1) * dy) / len2;
  return std::clamp(t, 0.0, 1.0);
}

static double pointToSeg(const Intersection &p, const Intersection &a,
                         const Intersection &b) {
  auto [x0, y0] = p.getPosition();
  auto [x1, y1] = a.getPosition();
  auto [x2, y2] = b.getPosition();
  double dx = x2 - x1, dy = y2 - y1;
  double len2 = dx * dx + dy * dy;
  double t = (len2 > 0.0) ? ((x0 - x1) * dx + (y0 - y1) * dy) / len2 : 0.0;
  t = std::clamp(t, 0.0, 1.0);
  double px = x1 + t * dx, py = y1 + t * dy;
  return std::hypot(x0 - px, y0 - py);
}

//--------------------------------------------------------------------------------
// Ramer–Douglas–Peucker polyline simplification
//--------------------------------------------------------------------------------

static double perpDistance(const Intersection &p, const Intersection &a,
                           const Intersection &b) {
  // exactly the same as pointToSeg
  return pointToSeg(p, a, b);
}

static void simplifyRDP(const std::vector<Intersection> &pts, double eps,
                        std::vector<Intersection> &out) {
  if (pts.size() < 3) {
    // nothing to simplify
    out = pts;
    return;
  }

  // find index of point furthest from AB
  double maxDist = 0.0;
  size_t idx = 0;
  for (size_t i = 1; i + 1 < pts.size(); ++i) {
    double d = perpDistance(pts[i], pts.front(), pts.back());
    if (d > maxDist) {
      maxDist = d;
      idx = i;
    }
  }

  if (maxDist > eps) {
    // keep this point, recurse on both halves
    std::vector<Intersection> left, right;
    simplifyRDP({pts.begin(), pts.begin() + idx + 1}, eps, left);
    simplifyRDP({pts.begin() + idx, pts.end()}, eps, right);

    // merge (drop duplicate at join)
    out.assign(left.begin(), left.end() - 1);
    out.insert(out.end(), right.begin(), right.end());
  } else {
    // all intermediate pts are within eps -> just AB
    out = {pts.front(), pts.back()};
  }
}

//--------------------------------------------------------------------------------
// generate()
//--------------------------------------------------------------------------------

MotorwayGenerator::MotorwayGenerator(double thresholdRatio, int defaultSpeed,
                                     int capacity)
    : thresholdRatio_(thresholdRatio), defaultSpeed_(defaultSpeed),
      capacity_(capacity) {}

void MotorwayGenerator::generate(Graph<Intersection, Road> &graph) {
  auto nodes = graph.getNodes();
  size_t n = nodes.size();
  if (n < 2)
    return;

  // 1) find farthest A,B
  size_t i0 = 0, i1 = 1;
  double maxD = 0;
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i + 1; j < n; ++j) {
      double d = euclid(nodes[i], nodes[j]);
      if (d > maxD) {
        maxD = d;
        i0 = i;
        i1 = j;
      }
    }
  const Intersection &A = nodes[i0];
  const Intersection &B = nodes[i1];

  // 2) dynamic threshold = ratio * AB length
  double threshold = maxD * thresholdRatio_;

  // 3) collect all in‐band nodes + A/B, with projection parameter
  std::vector<std::pair<double, size_t>> seq;
  seq.reserve(n);
  seq.emplace_back(0.0, i0);
  for (size_t k = 0; k < n; ++k) {
    if (k == i0 || k == i1)
      continue;
    if (pointToSeg(nodes[k], A, B) <= threshold) {
      double t = projectionParameter(nodes[k], A, B);
      seq.emplace_back(t, k);
    }
  }
  seq.emplace_back(1.0, i1);

  // 4) sort by t and build raw path
  std::sort(seq.begin(), seq.end(),
            [](auto &a, auto &b) { return a.first < b.first; });
  std::vector<Intersection> rawPath;
  rawPath.reserve(seq.size());
  for (auto &pr : seq)
    rawPath.push_back(nodes[pr.second]);

  // 5) simplify path with RDP epsilon = threshold/2
  std::vector<Intersection> smooth;
  simplifyRDP(rawPath, threshold * 0.5, smooth);

  // 6) connect each neighbor in smooth[]
  for (size_t i = 0; i + 1 < smooth.size(); ++i) {
    const auto &P = smooth[i];
    const auto &Q = smooth[i + 1];
    graph.addEdgeIfNotExists(Road(P, Q, defaultSpeed_, capacity_));
    graph.addEdgeIfNotExists(Road(Q, P, defaultSpeed_, capacity_));
  }
}
