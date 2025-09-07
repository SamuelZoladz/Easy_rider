/**
 * @file SfmlSettingsWindow.h
 * @brief SFML window with a simulation settings.
 *
 * Responsibilities:
 *  - Own a small SFML window.
 *  - Expose a single-frame tick() that processes events and renders.
 *  - Notify the host when the window opens/closes (e.g., pause/resume sim).
 */
#ifndef SFML_SETTINGS_WINDOW_H
#define SFML_SETTINGS_WINDOW_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>

class SfmlSettingsWindow {
public:
  /**
   * @brief Optional hooks invoked when the settings window opens/closes.
   *
   * Typical usage:
   *  - onOpen  → pause simulation.
   *  - onClose → resume simulation.
   */
  struct Callbacks {
    std::function<void()> onOpen;  ///< Called right before the window is shown.
    std::function<void()> onClose; ///< Called after the window has closed.
  };

  /**
   * @brief Construct settings window UI.
   * @param uiFont  Reference to a font used for labels.
   * @param cbs     Optional open/close callbacks.
   */
  explicit SfmlSettingsWindow(const sf::Font &uiFont, Callbacks cbs = {});
  ~SfmlSettingsWindow();

  SfmlSettingsWindow(const SfmlSettingsWindow &) = delete;
  SfmlSettingsWindow &operator=(const SfmlSettingsWindow &) = delete;
  SfmlSettingsWindow(SfmlSettingsWindow &&) = delete;
  SfmlSettingsWindow &operator=(SfmlSettingsWindow &&) = delete;

  //===--------------------------------------------------------------------===//
  // Window control
  //===--------------------------------------------------------------------===//

  /// Open the settings window (no-op if already open). Triggers onOpen().
  void open();

  /// Close the settings window if open. Triggers onClose().
  void close();

  /// @return true if the window exists and is currently open.
  [[nodiscard]] bool isOpen() const;

  /**
   * @brief Process one frame: handle events and render.
   *
   * Safe to call every frame from the host loop; does nothing if not open.
   */
  void tick();

private:
  /// Poll and handle SFML events (close, mouse input, dragging).
  void processEvents_();

  /// Render the current UI (labels + logarithmic speed slider).
  void render_();

  std::unique_ptr<sf::RenderWindow>
      win_;              ///< Owned auxiliary window (nullable).
  const sf::Font &font_; ///< UI font.
  Callbacks cbs_;        ///< Open/close hooks.

  bool dragging_ = false;       ///< True while the speed knob is being dragged.
  const float speedMin_ = 0.1f; ///< Slider min (log scale).
  const float speedMax_ = 10.f; ///< Slider max (log scale).
};

#endif // SFML_SETTINGS_WINDOW_H
