/**
 * @file SfmlStatsPanel.h
 * @brief Lightweight SFML overlay panel that displays simulation statistics.
 *
 * Responsibilities:
 *  - Reserve a right-side strip in the window and render a simple HUD.
 *  - Draw basic metrics (e.g., simulation time, average speed).
 *
 * Coordinate system: all sizes are in pixels; text is drawn in window space.
 */
#ifndef SFML_STATS_PANEL_H
#define SFML_STATS_PANEL_H

#include <SFML/Graphics.hpp>

/**
 * @brief Snapshot of values presented in the stats panel.
 */
struct StatsSnapshot {
  /// Simulation time in seconds (virtual time).
  double simTimeSec = 0.0;

  /// Average vehicle speed in world units per second.
  double avgSpeed = 0.0;

  /// Total time saved by rerouting in seconds.
  double rerouteSavedTime = 0.0;

  /// Total number of reroutes performed.
  int rerouteCounts = 0;
};

/**
 * @class SfmlStatsPanel
 * @brief Renders a minimalistic statistics panel on top of the SFML target.
 *
 * Usage:
 *  1) setWidth(), setHeight() to define occupied area.
 *  2) setFont() to provide a UI font.
 *  3) draw() every frame with current window size and StatsSnapshot.
 */
class SfmlStatsPanel {
public:
  /// Set the panel width (pixels). Defaults to 100px.
  void setWidth(float w);

  /// Provide a font for labels.
  void setFont(const sf::Font *font);

  /// Set the top bar (panel) height (pixels) if the panel uses a header area.
  void setHeight(float h);

  /**
   * @brief Draw the stats panel and texts.
   * @param rt         Render target (window or texture).
   * @param windowSize Full window size in pixels (for layout).
   * @param stats      Values to present.
   */
  void draw(sf::RenderTarget &rt, const sf::Vector2u &windowSize,
            const StatsSnapshot &stats) const;

private:
  float width_{100.f};            ///< Right-side panel width in pixels.
  float topBarH_{0.f};            ///< Optional top header height in pixels.
  const sf::Font *font_{nullptr}; ///< Font for UI text (external lifetime).

  /// Draws the panel background & separators.
  void drawPanel_(sf::RenderTarget &rt, const sf::Vector2u &sz) const;

  /// Draws the textual statistics.
  void drawTexts_(sf::RenderTarget &rt, const sf::Vector2u &sz,
                  const StatsSnapshot &stats) const;
};

#endif // SFML_STATS_PANEL_H
