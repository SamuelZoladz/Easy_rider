/**
 * @file Edge.h
 * @brief Declaration of the Edge class representing a connection between two
 * Nodes.
 */
#ifndef EDGE_H
#define EDGE_H

#include "Node.h"

/**
 * @class Edge
 * @brief Represents an edge connecting two Nodes in 2D space, with a computed
 * length and a maximum speed.
 */
class Edge {
public:
  /**
   * @brief Default constructor.
   *        Initializes both endpoints at (0,0), length to 0.0, and maxSpeed to
   * 0.
   */
  Edge();

  /**
   * @brief Parameterized constructor.
   * @param from The starting Node.
   * @param to The ending Node.
   * @param maxSpeed The maximum allowed speed on this edge.
   *
   * Automatically computes the Euclidean length between \p from and \p to.
   */
  Edge(const Node &from, const Node &to, int maxSpeed);

  /**
   * @brief Retrieves the starting Node.
   * @return A copy of the 'from' Node.
   */
  Node getFrom() const;

  /**
   * @brief Retrieves the ending Node.
   * @return A copy of the 'to' Node.
   */
  Node getTo() const;

  /**
   * @brief Retrieves the computed length of the edge.
   * @return The Euclidean distance between 'from' and 'to'.
   */
  double getLength() const;

  /**
   * @brief Retrieves the maximum speed for this edge.
   * @return The maximum speed (int).
   */
  int getMaxSpeed() const;

private:
  Node from_;     /**< The starting node. */
  Node to_;       /**< The ending node. */
  double length_; /**< Computed Euclidean distance between from_ and to_. */
  int maxSpeed_;  /**< Maximum speed allowed along this edge. */

  /**
   * @brief Computes and updates length_ as the distance between from_ and to_.
   */
  void computeLength();
};

#endif // EDGE_H
