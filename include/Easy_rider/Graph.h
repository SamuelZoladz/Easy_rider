/**
 * @file Graph.h
 * @brief A generic Graph template parameterized on Node and Edge types.
 *
 * Defines two concepts (NodeConcept, EdgeConcept) to constrain the
 * template parameters and implements a Graph that stores nodes and edges,
 * with a convenience method to add an edge only if it does not already exist.
 */
#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <concepts>
#include <utility>
#include <vector>

/**
 * @brief Concept that a Node type must satisfy.
 *
 * A Node must provide:
 *   - int getX()
 *   - int getY()
 *   - std::pair<int,int> getPosition()
 */
template <typename T>
concept NodeConcept = requires(const T &n) {
  { n.getX() } -> std::same_as<int>;
  { n.getY() } -> std::same_as<int>;
  { n.getPosition() } -> std::same_as<std::pair<int, int>>;
};

/**
 * @brief Concept that an Edge type must satisfy, given a Node type T.
 *
 * An Edge must provide:
 *   - T getFrom()
 *   - T getTo()
 *   - double getLength()
 *   - int getMaxSpeed()
 */
template <typename U, typename T>
concept EdgeConcept = requires(const U &e) {
  { e.getFrom() } -> std::same_as<T>;
  { e.getTo() } -> std::same_as<T>;
  { e.getLength() } -> std::same_as<double>;
  { e.getMaxSpeed() } -> std::same_as<int>;
};

/**
 * @class Graph
 * @brief A templated graph storing nodes of type T and edges of type U.
 *
 * @tparam T A Node type satisfying NodeConcept.
 * @tparam U An Edge type satisfying EdgeConcept<U,T>.
 */
template <NodeConcept T, typename U>
  requires EdgeConcept<U, T>
class Graph {
public:
  /**
   * @brief Default-constructs an empty graph.
   */
  Graph() = default;

  /**
   * @brief Add a node to the graph.
   * @param node The node to add.
   */
  void addNode(const T &node) { nodes_.push_back(node); }

  /**
   * @brief Add an edge to the graph without checking for duplicates.
   * @param edge The edge to add.
   */
  void addEdge(const U &edge) { edges_.push_back(edge); }

  /**
   * @enum    AddEdgeResult
   * @brief   Result of attempting to insert an edge with checks.
   */
  enum class AddEdgeResult {
    Success,       /**< Edge was inserted. */
    AlreadyExists, /**< Duplicate edge. */
    Crosses        /**< Would cross/overlap an existing edge. */
  };

  /**
   * @brief  Attempt to add a directed edge, rejecting duplicates or crossings.
   *
   * @param edge The edge to add.
   * @return
   *   - AddEdgeResult::AlreadyExists if an identical from->to edge is present.
   *   - AddEdgeResult::Crosses       if it would intersect or overlap any
   * existing edge (excluding shared endpoints).
   *   - AddEdgeResult::Success       otherwise (and the edge is inserted).
   */
  AddEdgeResult addEdgeIfNotExists(const U &edge) {
    auto f = edge.getFrom().getPosition();
    auto t = edge.getTo().getPosition();

    if (isDuplicate(f, t))
      return AddEdgeResult::AlreadyExists;

    if (crossesAnyEdge(f, t))
      return AddEdgeResult::Crosses;

    edges_.push_back(edge);
    return AddEdgeResult::Success;
  }

  /**
   * @brief Retrieve all nodes in the graph.
   * @return A vector of nodes.
   */
  std::vector<T> getNodes() const { return nodes_; }

  /**
   * @brief Retrieve all edges in the graph.
   * @return A vector of edges.
   */
  std::vector<U> getEdges() const { return edges_; }

private:
  std::vector<T> nodes_; /**< Stored nodes. */
  std::vector<U> edges_; /**< Stored edges. */

  /**
   * @brief  Compute the 2D orientation (cross product) of the triplet (A, B,
   * C).
   *
   * @param A  First point (std::pair<int,int>).
   * @param B  Second point.
   * @param C  Third point.
   * @return
   *   Positive if ABC is counter-clockwise, negative if clockwise, zero if
   * colinear.
   */
  static long long orient(const std::pair<int, int> &A,
                          const std::pair<int, int> &B,
                          const std::pair<int, int> &C) {
    return static_cast<long long>(B.first - A.first) * (C.second - A.second) -
           static_cast<long long>(B.second - A.second) * (C.first - A.first);
  }

  /**
   * @brief  Check if point C lies on the segment AB.
   *
   * @param A  One endpoint of the segment (std::pair<int,int>).
   * @param B  Other endpoint of the segment.
   * @param C  Point to test.
   * @return  True if C is between A and B (inclusive) in both x and y.
   */
  static bool onSegment(const std::pair<int, int> &A,
                        const std::pair<int, int> &B,
                        const std::pair<int, int> &C) {
    return std::min(A.first, B.first) <= C.first &&
           C.first <= std::max(A.first, B.first) &&
           std::min(A.second, B.second) <= C.second &&
           C.second <= std::max(A.second, B.second);
  }

  /**
   * @brief  Check if the directed segment f->t is a duplicate of an existing
   * one.
   *
   * @param f  Source point of new edge (std::pair<int,int>).
   * @param t  Target point of new edge.
   * @return  True if an edge with the same from/to already exists.
   */
  bool isDuplicate(const std::pair<int, int> &f,
                   const std::pair<int, int> &t) const {
    return std::any_of(edges_.begin(), edges_.end(), [&](auto const &e) {
      auto p = e.getFrom().getPosition();
      auto q = e.getTo().getPosition();
      return (p == f) && (q == t);
    });
  }

  /**
   * @brief  Test whether segment f->t crosses or overlaps segment q1->q2.
   *
   * Shared endpoints are considered non-crossing.
   *
   * @param f   Source of the new edge (std::pair<int,int>).
   * @param t   Target of the new edge.
   * @param q1  Source of an existing edge.
   * @param q2  Target of an existing edge.
   * @return   True if the two segments intersect or overlap.
   */
  bool segmentCrosses(const std::pair<int, int> &f,
                      const std::pair<int, int> &t,
                      const std::pair<int, int> &q1,
                      const std::pair<int, int> &q2) const {
    if (q1 == f || q1 == t || q2 == f || q2 == t)
      return false;

    long long o1 = orient(f, t, q1);
    long long o2 = orient(f, t, q2);
    long long o3 = orient(q1, q2, f);
    long long o4 = orient(q1, q2, t);

    if (((o1 > 0 && o2 < 0) || (o1 < 0 && o2 > 0)) &&
        ((o3 > 0 && o4 < 0) || (o3 < 0 && o4 > 0)))
      return true;

    if (o1 == 0 && onSegment(f, t, q1))
      return true;
    if (o2 == 0 && onSegment(f, t, q2))
      return true;
    if (o3 == 0 && onSegment(q1, q2, f))
      return true;
    if (o4 == 0 && onSegment(q1, q2, t))
      return true;

    return false;
  }

  /**
   * @brief  Determine if the new segment f->t crosses any edge in the graph.
   *
   * @param f  Source point of the new edge (std::pair<int,int>).
   * @param t  Target point of the new edge.
   * @return     True if it crosses or overlaps at least one existing edge.
   */
  bool crossesAnyEdge(const std::pair<int, int> &f,
                      const std::pair<int, int> &t) const {
    return std::any_of(edges_.begin(), edges_.end(), [&](auto const &e) {
      auto q1 = e.getFrom().getPosition();
      auto q2 = e.getTo().getPosition();
      return segmentCrosses(f, t, q1, q2);
    });
  }
};

#endif // GRAPH_H
