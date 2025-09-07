#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include "Easy_rider/Parameters/Parameters.h"
#include "Easy_rider/Visualizers/SfmlSettingsWindow.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <string>

SfmlSimulationVisualizer::SfmlSimulationVisualizer() = default;
SfmlSimulationVisualizer::~SfmlSimulationVisualizer() = default;

void SfmlSimulationVisualizer::processEvents() {
  if (!window_)
    return;

  sf::Event ev{};
  while (window_->pollEvent(ev)) {
    switch (ev.type) {
    case sf::Event::Closed: {
      if (settingsWindow_ && settingsWindow_->isOpen()) {
        settingsWindow_->close();
      }
      window_->close();
      break;
    }

    case sf::Event::Resized: {
      updateSceneViewport();
      break;
    }

    case sf::Event::MouseButtonPressed: {
      if (ev.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2f ui = window_->mapPixelToCoords(
            {ev.mouseButton.x, ev.mouseButton.y}, uiView_);
        handleUiClick(ui.x, ui.y);
      }
      break;
    }

    default:
      break;
    }
  }
}

void SfmlSimulationVisualizer::renderTo(sf::RenderTarget &target) {
  drawGraph(target);
  drawVehicles(target);
}

void SfmlSimulationVisualizer::onSimulationAttached() {
  graphCacheDirty_ = true;
}

void SfmlSimulationVisualizer::run() { SimulationVisualizer::run(); }

bool SfmlSimulationVisualizer::isOpen() const {
  return window_ && window_->isOpen();
}

void SfmlSimulationVisualizer::pause() {
  if (!paused_) {
    paused_ = true;
    prevTimeScale_ = timeScale_;
    timeScale_ = 0.0;
  }
}

void SfmlSimulationVisualizer::resume() {
  if (paused_) {
    paused_ = false;
    timeScale_ = (prevTimeScale_ > 0.0 ? prevTimeScale_ : 1.0);
  }
}

void SfmlSimulationVisualizer::restart() {
  // TODO: full restart of a new simulation session
  simulation_->start();
}

void SfmlSimulationVisualizer::stop() {
  // Stops visualization by closing the window.
  closeWindow();
}

void SfmlSimulationVisualizer::updateSceneViewport() {
  auto size = window_->getSize();
  const float windowW = static_cast<float>(size.x);
  const float windowH = static_cast<float>(size.y);

  // Layout: top area = scene, bottom area = UI bar.
  const float bottomUiHeightPx =
      std::min(Parameters::uiBottomHeight(), windowH);
  const float topContentHeightPx = std::max(0.f, windowH - bottomUiHeightPx);
  const float topContentHeightFrac =
      (windowH > 0.f) ? (topContentHeightPx / windowH) : 0.f;

  // Scene view occupies the top portion.
  sceneView_.reset(sf::FloatRect(0.f, 0.f, windowW, topContentHeightPx));
  sceneView_.setViewport(sf::FloatRect(0.f, 0.f, 1.f, topContentHeightFrac));

  // UI view (bottom bar) occupies the remaining portion.
  uiView_.reset(sf::FloatRect(0.f, 0.f, windowW, bottomUiHeightPx));
  uiView_.setViewport(sf::FloatRect(0.f, topContentHeightFrac, 1.f,
                                    1.f - topContentHeightFrac));

  layoutUi();

  // Margins (in screen pixels) used by positioning code.
  uiMargins_.left = 0.f;
  uiMargins_.top = 0.f;
  uiMargins_.right = Parameters::statsPanelWidth();
  uiMargins_.bottom = Parameters::uiBottomHeight();

  graphCacheDirty_ = true;
}

void SfmlSimulationVisualizer::setView(const VisualizerView &view) {
  SimulationVisualizer::setView(view);
  graphCacheDirty_ = true;
}

void SfmlSimulationVisualizer::openWindow(std::uint32_t width,
                                          std::uint32_t height,
                                          const std::string &title) {
  window_ = std::make_unique<sf::RenderWindow>(
      sf::VideoMode(width, height), title,
      sf::Style::Titlebar | sf::Style::Close);
  window_->setFramerateLimit(Parameters::frameRateLimit());

  initUiIfNeeded();
  uiView_ = window_->getDefaultView();
  sceneView_ = uiView_;

  // Settings sub-window (pauses simulation on open, restores on close).
  settingsWindow_ = std::make_unique<SfmlSettingsWindow>(
      uiFont_, SfmlSettingsWindow::Callbacks{
                   [this]() noexcept(noexcept(this->pause())) {
                     pausedBeforeSettings_ = paused_;
                     if (!paused_)
                       pause();
                   },
                   [this]() noexcept(noexcept(this->resume())) {
                     if (!pausedBeforeSettings_ && paused_) {
                       resume();
                     }
                   }});

  // Stats panel setup.
  statsPanel_.setFont(&uiFont_);
  statsPanel_.setWidth(Parameters::statsPanelWidth());
  statsPanel_.setHeight(uiTopBarHeight_);

  updateSceneViewport();
  layoutUi();
}

void SfmlSimulationVisualizer::openSettings() const {
  if (settingsWindow_)
    settingsWindow_->open();
}

void SfmlSimulationVisualizer::closeWindow() {
  if (window_) {
    window_->close();
    window_.reset();
  }
}

void SfmlSimulationVisualizer::renderFrame() {
  if (!window_)
    return;

  processEvents();

  const sf::Color bg = Parameters::argb(Parameters::backgroundColor());
  window_->clear(bg);

  // 1) Scene (graph + stats)
  window_->setView(sceneView_);
  renderTo(*window_);
  drawStats(*window_);

  // 2) UI (screen-space bottom bar, buttons)
  window_->setView(uiView_);
  drawUi(*window_);

  window_->display();

  // Tick the settings popup if present.
  if (settingsWindow_)
    settingsWindow_->tick();
}

void SfmlSimulationVisualizer::drawStats(sf::RenderTarget &rt) {
  // Gather simulation stats for the overlay panel.
  StatsSnapshot snap;
  snap.simTimeSec = simulation_->getSimTime();
  snap.avgSpeed = simulation_->averageSpeed();

  const sf::Vector2u sz = window_->getSize();
  const float windowH = static_cast<float>(sz.y);
  const float topContentHeightPx =
      std::max(0.f, windowH - Parameters::uiBottomHeight());

  // The stats panel occupies the right side across the scene area height.
  uiTopBarHeight_ = topContentHeightPx;
  statsPanel_.setHeight(topContentHeightPx);
  statsPanel_.setWidth(Parameters::statsPanelWidth());

  // Update margins so graph/layout can account for the panel.
  uiMargins_.right = Parameters::statsPanelWidth();
  uiMargins_.bottom = Parameters::uiBottomHeight();

  statsPanel_.draw(rt, sz, snap);
}
