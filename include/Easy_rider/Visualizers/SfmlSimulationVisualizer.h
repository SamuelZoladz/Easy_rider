/**
 * @file SfmlSimulationVisualizer.h
 * @brief SFML-based visualizer that renders the traffic graph, vehicles and UI.
 *
 * Responsibilities:
 *  - Window lifecycle (open/close, event loop).
 *  - Scene rendering (graph, vehicles) and UI panels (buttons, stats).
 *  - Simulation control helpers (pause/resume/restart).
 */
#ifndef SFML_SIMULATION_VISUALIZER_H
#define SFML_SIMULATION_VISUALIZER_H

#include "SfmlSettingsWindow.h"
#include "SfmlStatsPanel.h"
#include "SimulationVisualizer.h"
#include "VisualizerUtils.h"

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/**
 * @class SfmlSimulationVisualizer
 * @brief Concrete implementation of SimulationVisualizer backed by SFML.
 */
class SfmlSimulationVisualizer final : public SimulationVisualizer {
public:
  SfmlSimulationVisualizer();
  ~SfmlSimulationVisualizer() override;

  SfmlSimulationVisualizer(const SfmlSimulationVisualizer &) = delete;
  SfmlSimulationVisualizer &
  operator=(const SfmlSimulationVisualizer &) = delete;
  SfmlSimulationVisualizer(SfmlSimulationVisualizer &&) = delete;
  SfmlSimulationVisualizer &operator=(SfmlSimulationVisualizer &&) = delete;

  /**
   * @brief Render a single frame: process events, draw scene & UI, and display.
   *
   * Calls processEvents(), then performs rendering to the internal window.
   * No-op if the window is not open.
   */
  void renderFrame() override;

  /**
   * @brief Set the logical view (world origin & scale) used for world->screen.
   */
  void setView(const VisualizerView &view) override;

  /**
   * @brief Pause/unpause drawing of time-dependent elements.
   *        Equivalent to pause()/resume() helpers.
   */
  void setPaused(bool paused) override { paused_ = paused; }

  /**
   * @brief @return true if the SFML window exists and is open.
   */
  [[nodiscard]] bool isOpen() const override;

  /**
   * @brief Start the main loop (blocks until the window is closed).
   */
  void run() override;

  /**
   * @brief Create (or recreate) the SFML window if needed.
   * @param width  Window width in pixels.
   * @param height Window height in pixels.
   * @param title  Window title.
   */
  void openWindow(std::uint32_t width, std::uint32_t height,
                  const std::string &title);

  /**
   * @brief Close the window if it is open.
   */
  void closeWindow();

  /**
   * @brief Poll and handle window events (close, resize, mouse, keyboard).
   *        No-op if the window is not open.
   */
  void processEvents();

  /**
   * @brief Render the entire scene (graph, vehicles, UI) into the given target.
   * @note Used by renderFrame() for the on-screen window; can be used for
   *       off-screen targets in tests.
   */
  void renderTo(sf::RenderTarget &target);

  /**
   * @brief Draw the statistics panel to the given target and update UI margins.
   * @note Updates internal UI layout-related state (top/bottom panel heights).
   */
  void drawStats(sf::RenderTarget &target);

  /// Forces paused state (time scale -> 0.0), preserving the previous value.
  void pause();

  /// Restores the previous time scale (or 1.0 if unknown) and unpauses.
  void resume();

  /// Ensures a window exists and resumes if paused.
  void restart();

  /// Closes the window (stops visualization).
  void stop();

  /// Opens settings UI (modal/auxiliary window).
  void openSettings();

  /// @brief Node (intersection) circle radius in pixels.
  void setNodeRadius(float r) { nodeRadius_ = r; }

  /// @brief Vehicle dot radius in pixels.
  void setVehicleRadius(float r) { vehicleRadius_ = r; }

  /**
   * @brief Edge thickness in pixels (triangulated strips).
   *        Marks the cached graph geometry as dirty.
   */
  void setEdgeThickness(float t) {
    graphCacheDirty_ = true;
    edgeThickness_ = t;
  }

protected:
  /**
   * @brief Called after a simulation is attached; rebuilds caches if needed.
   */
  void onSimulationAttached() override;

private:
  /// Build (or rebuild) cached vertex arrays for graph edges & nodes.
  void rebuildGraphCache();

  /// Draw cached graph geometry (edges & nodes) into the target.
  void drawGraph(sf::RenderTarget &target);

  /// Draw vehicle markers into the target.
  void drawVehicles(sf::RenderTarget &target);

  /**
   * @brief Simple clickable rectangle button shown in the right-side panel.
   */
  struct UiButton {
    sf::FloatRect rect; ///< Button hitbox in window coordinates.
    std::string label;  ///< Static label; may be overridden when drawing.
  };

  /// Lazily initialize UI (loads fonts, prepares views) if not initialized.
  void initUiIfNeeded();

  /// Compute button rectangles based on current window size.
  void layoutUi();

  /// Draw the right-side panel and buttons (with dynamic labels).
  void drawUi(sf::RenderTarget &rt);

  /// Handle a UI click in window-space coordinates (dispatch actions).
  void handleUiClick(float mx, float my);

  /// Update sceneView_ viewport so the scene does not render under side panels.
  void updateSceneViewport();

  /// Compute scene placement and view scale from world positions & UI margins.
  void graphPositioning(const std::vector<sf::Vector2f> &positions);

  std::unique_ptr<sf::RenderWindow> window_; ///< Main window (nullable).
  sf::View sceneView_;                       ///< World content view.
  sf::View uiView_;                          ///< UI overlay view.

  std::unique_ptr<sf::VertexArray> edgesVA_; ///< Cached edge.
  std::vector<sf::Vector2f> nodesVA_;        ///< Cached node.

  std::unique_ptr<SfmlSettingsWindow> settingsWindow_; ///< Settings dialog.

  float statsPanelWidth_{100.0f}; ///< Stats sub-panel width on the right (px).
  float uiBottomHeight_{100.0f};  ///< Bottom UI bar height (px).
  float uiTopBarHeight_{0.0f};    ///< Top area occupied by stats (computed).
  UiMargins uiMargins_{};         ///< Left/Right/Top/Bottom outer margins (px).
  float contentPaddingPx_{12.0f}; ///< Inner padding for scene content (px).

  std::vector<UiButton> uiButtons_; ///< Clickable UI controls.
  sf::Font uiFont_;                 ///< UI font.
  bool uiFontLoaded_{false};        ///< True if uiFont_ was loaded.

  SfmlStatsPanel statsPanel_; ///< Stats drawer (owned component).

  float nodeRadius_{5.0f};    ///< Node marker radius (px).
  float vehicleRadius_{4.0f}; ///< Vehicle marker radius (px).
  float edgeThickness_{1.5f}; ///< Edge thickness (px).

  bool paused_{false};         ///< If true, time-dependent drawing is paused.
  bool graphCacheDirty_{true}; ///< If true, graph cache must be rebuilt.

  double prevTimeScale_{1.0};        ///< Stored time scale for resume().
  bool pausedBeforeSettings_{false}; ///< To restore pause state after settings.
};

#endif // SFML_SIMULATION_VISUALIZER_H
