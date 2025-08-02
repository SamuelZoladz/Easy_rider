#ifndef GRAPH_VISUALIZER_H
#define GRAPH_VISUALIZER_H

#include "Graph.h"
#include "Intersection.h"
#include "Road.h"
#include <SFML/Graphics.hpp>

/**
 * @brief Temporary test-only class to visualize
 *        a Graph<Intersection,Road> using SFML.
 */
class GraphVisualizer {
public:
  GraphVisualizer(unsigned int width = 800, unsigned int height = 600,
                  const std::string &title = "Graph Visualizer");

  void run(const Graph<Intersection, Road> &graph);

private:
  void processEvents();
  void draw(const Graph<Intersection, Road> &graph);

  sf::RenderWindow window_;
  sf::Font font_;
};

#endif // GRAPH_VISUALIZER_H
