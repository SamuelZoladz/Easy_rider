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
   * @brief Add an edge only if there is not already an edge between the same
   * nodes.
   *
   * Compares by positions of the endpoints (via getPosition()).
   *
   * @param edge The edge to add.
   * @return true if the edge was inserted; false if an equivalent edge already
   * existed.
   */
  bool addEdgeIfNotExists(const U &edge) {
    auto fromPos = edge.getFrom().getPosition();
    auto toPos = edge.getTo().getPosition();

    for (auto const &e : edges_) {
      if (e.getFrom().getPosition() == fromPos &&
          e.getTo().getPosition() == toPos) {
        return false;
      }
    }

    edges_.push_back(edge);
    return true;
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
};

#endif // GRAPH_H
