/**
 * @file Graph.h
 * @brief Directed, id-addressable graph template with node/edge concepts,
 * id->index lookup, and outgoing adjacency.
 *
 * @details
 * This header defines:
 *  - Concepts: NodeConcept and EdgeConcept that constrain the required
 * interface of T (node) and U (edge).
 *  - Class template Graph<T, U>: stores nodes and edges, maintains a fast
 * id->index map for nodes, and a per-node outgoing adjacency list keyed by node
 * id.
 *
 * Key properties:
 *  - Nodes are stored contiguously (std::vector<T>); each node supplies its id
 * via T::getId().
 *  - Edges are directed (fromId -> toId) and stored contiguously
 * (std::vector<U>).
 *  - Outgoing adjacency (unordered_map<int, vector<int>>) is updated on every
 * edge insertion.
 *  - Geometry helpers are provided internally to reject duplicate edges and
 * planar crossings in addEdgeIfNotExists().
 */
#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief Concept that a Node type must satisfy.
 *
 * A Node must provide:
 *   - int getId()
 *   - int getX()
 *   - int getY()
 *   - std::pair<int,int> getPosition()
 */
template <typename T>
concept NodeConcept = requires(const T &n) {
  { n.getId() } -> std::same_as<int>;
  { n.getX() } -> std::same_as<int>;
  { n.getY() } -> std::same_as<int>;
  { n.getPosition() } -> std::same_as<std::pair<int, int>>;
};

/**
 * @brief Concept that an Edge type must satisfy.
 *
 * An Edge must provide:
 *   - int getFromId()
 *   - int getToId()
 *   - double getLength()
 *   - int getMaxSpeed()
 */
template <typename U>
concept EdgeConcept = requires(const U &e) {
  { e.getFromId() } -> std::same_as<int>;
  { e.getToId() } -> std::same_as<int>;
  { e.getLength() } -> std::same_as<double>;
  { e.getMaxSpeed() } -> std::same_as<int>;
};

/**
 * @class Graph
 * @brief A templated graph storing nodes of type T and edges of type U.
 *
 * @tparam T A Node type satisfying NodeConcept.
 * @tparam U An Edge type satisfying EdgeConcept.
 */
template <NodeConcept T, typename U>
  requires EdgeConcept<U>
class Graph {
public:
  /**
   * @brief Default-constructs an empty graph.
   */
  Graph() = default;

  /**
   * @brief Add a node to the graph. Updates id to index map.
   * @param node The node to add.
   */
  void addNode(const T &node) {
    const int id = node.getId();
    nodeIndexById_[id] = nodes_.size();
    nodes_.push_back(node);
  }

  /**
   * @brief Add an edge to the graph without checks.
   * @param edge The edge to add.
   */
  void addEdge(const U &edge) {
    edges_.push_back(edge);
    const int uId = edge.getFromId();
    const int vId = edge.getToId();

    const int uIdx = static_cast<int>(indexOfId(uId));
    const int vIdx = static_cast<int>(indexOfId(vId));

    const std::size_t eIdx = edges_.size() - 1;
    outgoingIndex_[uIdx].emplace_back(vIdx, eIdx);
  }

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
   *                                  existing edge (excluding shared
   * endpoints).
   *   - AddEdgeResult::Success       otherwise (and the edge is inserted).
   */
  AddEdgeResult addEdgeIfNotExists(const U &edge) {
    const int f = edge.getFromId();
    const int t = edge.getToId();

    if (isDuplicate(f, t))
      return AddEdgeResult::AlreadyExists;

    const auto p = positionOf(f);
    const auto q = positionOf(t);
    if (crossesAnyEdge(p, q))
      return AddEdgeResult::Crosses;

    addEdge(edge);
    return AddEdgeResult::Success;
  }

  /**
   * @brief Get read-only access to the all nodes in the graph.
   * @return A const reference to the vector of nodes.
   */
  const std::vector<T> &getNodes() const { return nodes_; }

  /**
   * @brief Get read-only access to the all edges in the graph.
   * @return A const reference to the vector of edges.
   */
  const std::vector<U> &getEdges() const { return edges_; }

  /**
   * @brief Get node index by id.
   * @param id Node identifier.
   * @return Index into getNodes().
   * @warning Throws std::out_of_range if id is not present.
   */
  std::size_t indexOfId(int id) const { return nodeIndexById_.at(id); }

  /**
   * @brief Whether the graph contains a node with the given id.
   */
  bool hasId(int id) const {
    return nodeIndexById_.find(id) != nodeIndexById_.end();
  }

  /**
   * @brief Position lookup helper (id → {x,y}).
   * @param id Node identifier.
   * @return Pair {x,y} of the node position.
   */
  std::pair<int, int> positionOf(int id) const {
    const std::size_t idx = indexOfId(id);
    return nodes_[idx].getPosition();
  }

  /**
   * @brief Outgoing adjacency by node id (neighbor node ids).
   * @param id Source node id.
   * @return const reference to a vector of pair of neighbor node ids and edge
   * indexes. Empty if none.
   *
   * Maintained incrementally on every successful edge insertion.
   */
  using NeighborEdge = std::pair<int, std::reference_wrapper<const U>>;

  std::vector<NeighborEdge> outgoing(int uIdx) const {
    std::vector<NeighborEdge> result;
    auto it = outgoingIndex_.find(uIdx);
    if (it == outgoingIndex_.end())
      return result;

    const auto &lst = it->second;
    result.reserve(lst.size());
    for (const auto &p : lst) {
      const int vIdx = p.first;
      const std::size_t eIdx = p.second;
      result.emplace_back(vIdx, std::cref(edges_.at(eIdx)));
    }
    return result;
  }

  /**
   * @brief Accessor for the id→index map (read-only).
   */
  const std::unordered_map<int, std::size_t> &nodeIndexById() const {
    return nodeIndexById_;
  }

private:
  std::vector<T> nodes_; /**< Stored nodes. */
  std::vector<U> edges_; /**< Stored edges. */

  std::unordered_map<int, std::size_t> nodeIndexById_; /**< Fast id->index. */
  std::unordered_map<int, std::vector<std::pair<int, std::size_t>>>
      outgoingIndex_;

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
   * @brief Check if a directed edge (u->v) is present.
   */
  bool isDuplicate(int u, int v) const {
    return std::any_of(edges_.begin(), edges_.end(), [&](const U &e) {
      return e.getFromId() == u && e.getToId() == v;
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
  static bool segmentCrosses(const std::pair<int, int> &f,
                             const std::pair<int, int> &t,
                             const std::pair<int, int> &q1,
                             const std::pair<int, int> &q2) {
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
      auto q1 = positionOf(e.getFromId());
      auto q2 = positionOf(e.getToId());
      return segmentCrosses(f, t, q1, q2);
    });
  }
};

#endif // GRAPH_H
