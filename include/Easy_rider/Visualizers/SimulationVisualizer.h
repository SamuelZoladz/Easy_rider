/**
 * @file SimulationVisualizer.h
 * @brief Abstract base for simulation visualizers (text or graphics).
 *
 * Responsibilities:
 *  - Hold the view configuration (world -> screen).
 *  - Provide a renderFrame() contract for derived visualizers.
 *  - Offer a default run() loop that advances the Simulation.
 *  - Manage time scaling (speed-up / slow-down).
 */
#ifndef SIMULATION_VISUALIZER_H
#define SIMULATION_VISUALIZER_H

#include <Easy_rider/Simulation/Simulation.h>
#include <chrono>

/**
 * @brief View parameters used to transform world coordinates to screen space.
 */
struct VisualizerView {
  float originX{0.0f}; ///< World-space origin (x) mapped to screen offset.
  float originY{0.0f}; ///< World-space origin (y) mapped to screen offset.
  float scale{1.0f};   ///< World units -> pixels multiplier.
  float offsetX{0.0f}; ///< Screen-space offset (x), in pixels.
  float offsetY{0.0f}; ///< Screen-space offset (y), in pixels.
};

/**
 * @brief UI margins in pixels used by visualizers when laying out the scene.
 */
struct UiMargins {
  float left{0.0f};
  float right{0.0f};
  float top{0.0f};
  float bottom{0.0f};
};

/**
 * @class SimulationVisualizer
 * @brief Abstract facade that connects a Simulation with a rendering backend.
 *
 * Typical usage:
 *  1) Create a concrete visualizer (e.g., SFML-based).
 *  2) attachSimulation(sim) — ownership stays with the caller.
 *  3) setView(view) — optional, to control world->screen mapping.
 *  4) run() — enters the main loop while isOpen() is true.
 *
 * Derivations must implement renderFrame(), and usually override isOpen() and
 * run() (if a custom main loop is needed).
 */
class SimulationVisualizer {
public:
  virtual ~SimulationVisualizer() = default;

  /**
   * @brief Attach a Simulation instance to visualize (non-owning).
   *        Triggers onSimulationAttached() for cache rebuilds in derived
   * classes.
   */
  void attachSimulation(Simulation *sim) {
    simulation_ = sim;
    onSimulationAttached();
  }

  /**
   * @brief Configure the world -> screen mapping used during rendering.
   */
  virtual void setView(const VisualizerView &view) { view_ = view; }

  /**
   * @brief Render a single frame (pure virtual).
   *
   * Implementations typically:
   *  - process window/input events,
   *  - draw the scene (graph, actors),
   *  - draw overlays/UI,
   */
  virtual void renderFrame() = 0;

  /**
   * @brief Optional pause flag hook.
   */
  virtual void setPaused(bool /*paused*/) {}

  /**
   * @brief Backend open-state for the run() loop. Return true while active.
   */
  [[nodiscard]] virtual bool isOpen() const { return false; }

  /**
   * @brief Drive the Simulation and rendering using wall-clock delta time.
   *
   * Uses a variable-step integrator:
   *   dtSim = wallSecondsSinceLastFrame * timeScale().
   *
   * The default implementation:
   *  - starts the simulation if attached,
   *  - loops while isOpen(),
   *  - advances the simulation,
   *  - calls renderFrame().
   */
  virtual void run() {
    using clock = std::chrono::steady_clock;

    auto last = clock::now();

    if (simulation_) {
      simulation_->start();
    }

    while (isOpen()) {
      const auto now = clock::now();
      const double wall = std::chrono::duration<double>(now - last).count();
      last = now;

      if (simulation_) {
        simulation_->update(wall * timeScale_);
      }

      renderFrame();
    }
  }

  /**
   * @brief Set simulation time scaling factor (1.0 → real time).
   *        Examples: 2.0 -> 2× faster, 0.5 -> half speed, 0.0 -> paused.
   *        Negative values are clamped to 0.0.
   */
  void setTimeScale(double s) { timeScale_ = (s > 0.0 ? s : 0.0); }

  /// @brief Current time scale factor.
  [[nodiscard]] double timeScale() const { return timeScale_; }

  /// @brief Return the currently attached simulation (may be nullptr).
  Simulation *simulation() { return simulation_; }

  /// @brief Const overload for the attached simulation (may be nullptr).
  const Simulation *simulation() const { return simulation_; }

protected:
  /**
   * @brief Called after attachSimulation(); override to rebuild caches, etc.
   */
  virtual void onSimulationAttached() {}

  Simulation *simulation_{nullptr}; ///< Non-owning pointer to the simulation.
  VisualizerView view_{};           ///< World->screen mapping in effect.
  double timeScale_{1.0};           ///< Time scaling factor (>= 0.0).
};

#endif // SIMULATION_VISUALIZER_H
