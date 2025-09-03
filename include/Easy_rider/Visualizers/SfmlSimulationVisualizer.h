/**
 * @file SfmlSimulationVisualizer.h
 * @brief SFML-based visualizer that draws the graph and moving vehicles.
 */
#ifndef SFML_SIMULATION_VISUALIZER_H
#define SFML_SIMULATION_VISUALIZER_H

#include "SimulationVisualizer.h"
#include "VisualizerUtils.h"

#include <SFML/Graphics.hpp>

#include <functional>
#include <memory>
#include <string>

class SfmlSimulationVisualizer final : public SimulationVisualizer {
public:
  SfmlSimulationVisualizer();
  ~SfmlSimulationVisualizer() override;

  void renderFrame() override;
  void resize(std::uint32_t width, std::uint32_t height) override;
  void setView(const VisualizerView &view) override;
  void setPaused(bool paused) override { paused_ = paused; }
  bool isOpen() const override;
  void run(double targetFps = 60.0) override;

  void openWindow(std::uint32_t width, std::uint32_t height,
                  const std::string &title);
  void closeWindow();
  void processEvents();

  void renderTo(sf::RenderTarget &target);

  void setGraphProvider(std::function<viz::GraphDrawData()> provider);
  void setVehicleProvider(std::function<std::vector<viz::Vec2>()> provider);

  void setNodeRadius(float r) { nodeRadius_ = r; }
  void setVehicleRadius(float r) { vehicleRadius_ = r; }
  void setEdgeThickness(float t) { edgeThickness_ = t; }

protected:
  void onSimulationAttached() override;

private:
  void rebuildGraphCache();
  void drawGraph(sf::RenderTarget &target);
  void drawVehicles(sf::RenderTarget &target);

  std::function<viz::GraphDrawData()> graphProvider_;
  std::function<std::vector<viz::Vec2>()> vehicleProvider_;

  std::unique_ptr<sf::RenderWindow> window_;

  std::unique_ptr<sf::VertexArray> edgesVA_;
  std::unique_ptr<sf::VertexArray> nodesVA_;

  float nodeRadius_{5.0f};
  float vehicleRadius_{4.0f};
  float edgeThickness_{1.5f};

  bool paused_{false};
  bool graphCacheDirty_{true};
};

#endif // SFML_SIMULATION_VISUALIZER_H
