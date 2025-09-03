/**
 * @file SimulationVisualizer.h
 * @brief Abstract base for simulation visualizers (text or graphics).
 *
 * Provides:
 *  - view configuration (world -> screen),
 *  - a renderFrame() contract,
 *  - a run() loop that advances the Simulation at a target FPS,
 *  - time scaling (speed-up / slow-down).
 */
#ifndef SIMULATION_VISUALIZER_H
#define SIMULATION_VISUALIZER_H

#include "Easy_rider/Simulation/Simulation.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

/**
 * @brief View definition used by visualizers to transform world -> screen.
 */
struct VisualizerView {
  float originX{0.0f};
  float originY{0.0f};
  float scale{1.0f};
};

class SimulationVisualizer {
public:
  virtual ~SimulationVisualizer() = default;

  /**
   * @brief Attach simulation to visualize. The visualizer does not own it.
   */
  void attachSimulation(Simulation *sim) {
    simulation_ = sim;
    onSimulationAttached();
  }

  /**
   * @brief Configure world -> screen mapping used during rendering.
   */
  virtual void setView(const VisualizerView &view) { view_ = view; }

  /**
   * @brief Render a single frame.
   */
  virtual void renderFrame() = 0;

  /**
   * @brief Resize output surface if applicable.
   */
  virtual void resize(std::uint32_t /*width*/, std::uint32_t /*height*/) {}

  /**
   * @brief Mark paused state.
   */
  virtual void setPaused(bool paused) {}

  /**
   * @brief Backend open-state for the run() loop. Return true while active.
   *
   */
  virtual bool isOpen() const { return false; }

  /**
   * @brief Drive the Simulation and rendering at a target FPS.
   *
   * Uses a variable-step integrator: dtSim = realDeltaSeconds * timeScale().
   * The loop sleeps to cap the frame rate to @p targetFps (default 60).
   *
   * @param targetFps Desired frames per second (min clamped to 1.0).
   */
  virtual void run(double targetFps = 60.0) {
    using clock = std::chrono::steady_clock;
    if (targetFps < 1.0)
      targetFps = 1.0;
    const double frameTarget = 1.0 / targetFps;

    auto last = clock::now();
    simulation_->start();
    while (isOpen()) {
      const auto t0 = clock::now();
      double wall = std::chrono::duration<double>(t0 - last).count();
      if (wall > 0.25)
        wall = 0.25;
      last = t0;
      if (simulation_) {
        simulation_->update(wall * timeScale_);
      }
      renderFrame();
      const auto t1 = clock::now();
      const double spent = std::chrono::duration<double>(t1 - t0).count();
      const double sleepSec = frameTarget - spent;
      if (sleepSec > 0.0) {
        std::this_thread::sleep_for(std::chrono::duration<double>(sleepSec));
      }
    }
  }

  /**
   * @brief Set simulation time scaling factor (1.0 -> real time).
   * E.g. 2.0 -> 2x faster, 0.5 -> half speed.
   */
  void setTimeScale(double s) { timeScale_ = (s > 0.0 ? s : 0.0); }

  /**
   * @brief Current time scale factor.
   */
  double timeScale() const { return timeScale_; }

  /**
   * @brief Return the currently attached simulation (may be nullptr).
   */
  Simulation *simulation() { return simulation_; }

protected:
  /**
   * @brief Called after attachSimulation(); override to rebuild caches.
   */
  virtual void onSimulationAttached() {}

  Simulation *simulation_{nullptr};
  VisualizerView view_{};
  double timeScale_{1.0};
};

#endif // SIMULATION_VISUALIZER_H
