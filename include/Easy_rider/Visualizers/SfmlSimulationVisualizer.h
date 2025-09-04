/**
 * @file SfmlSimulationVisualizer.h
 * @brief SFML-based visualizer that draws the graph and moving vehicles.
 */
#ifndef SFML_SIMULATION_VISUALIZER_H
#define SFML_SIMULATION_VISUALIZER_H

#include "SfmlSettingsWindow.h"
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

  /**
   * @brief Render single frame: process events, draw scene UI, display.
   */
  void renderFrame() override;
  void setView(const VisualizerView &view) override;
  void setPaused(bool paused) override { paused_ = paused; }
  bool isOpen() const override;
  void run(double targetFps = 60.0) override;

  void openWindow(std::uint32_t width, std::uint32_t height,
                  const std::string &title);
  void closeWindow();

  /**
   * @brief Handle window events (close, mouse clicks, keyboard).
   */
  void processEvents();

  void renderTo(sf::RenderTarget &target);

  void setGraphProvider(std::function<viz::GraphDrawData()> provider);
  void setVehicleProvider(std::function<std::vector<viz::Vec2>()> provider);

  void setNodeRadius(float r) { nodeRadius_ = r; }
  void setVehicleRadius(float r) { vehicleRadius_ = r; }
  void setEdgeThickness(float t) { edgeThickness_ = t; }
  /// \brief Forces paused state (timeScale_ -> 0.0).
  void pause();

  /// \brief Resumes simulation restoring previous time scale (or 1.0).
  void resume();

  /// \brief Starts visualization: ensures window exists and resumes if paused.
  void restart();

  /// \brief Stops visualization by closing the window.
  void stop();

  void openSettings();

protected:
  void onSimulationAttached() override;

private:
  static sf::Vector2f toScreen(const viz::Vec2 &w, const VisualizerView &v) {
    return {(w.x - v.originX) * v.scale, (w.y - v.originY) * v.scale};
  }
  void rebuildGraphCache();
  void drawGraph(sf::RenderTarget &target);
  void drawVehicles(sf::RenderTarget &target);

  std::unique_ptr<SfmlSettingsWindow> settingsWindow_;

  /**
   * @brief Simple clickable rectangle button in the right-side panel.
   */
  struct UiButton {
    sf::FloatRect rect; ///< Button hitbox in window coordinates.
    std::string
        label; ///< Static label; Pause/Resume label is dynamic at draw time.
  };

  float panelWidth_{200.0f};
  std::vector<UiButton> uiButtons_;
  sf::Font uiFont_;
  bool uiFontLoaded_{false};

  double prevTimeScale_{1.0};

  /**
   * @brief Lazy UI initialization — calls layoutUi() when needed.
   */
  void initUiIfNeeded();

  /**
   * @brief Compute button rectangles based on the current window size.
   */
  void layoutUi();

  /**
   * @brief Draw the right-side panel and buttons (with dynamic labels).
   */
  void drawUi(sf::RenderTarget &rt);

  /**
   * @brief Handle a click in window coordinates; dispatch
   * Start/Pause-Resume/Stop.
   */
  void handleUiClick(float mx, float my);

  sf::View sceneView_;
  sf::View uiView_;

  /**
   * @brief Update sceneView_ viewport so the scene does not draw under the
   * right panel.
   */
  void updateSceneViewport();

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
