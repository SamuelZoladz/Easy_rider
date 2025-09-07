#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
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
    if (ev.type == sf::Event::Closed) {
      if (settingsWindow_ && settingsWindow_->isOpen())
        settingsWindow_->close();
      window_->close();
    } else if (ev.type == sf::Event::Resized) {
      updateSceneViewport();
      layoutUi();
    } else if (ev.type == sf::Event::MouseButtonPressed &&
               ev.mouseButton.button == sf::Mouse::Left) {
      sf::Vector2i pixel(ev.mouseButton.x, ev.mouseButton.y);
      sf::Vector2f ui = window_->mapPixelToCoords(pixel, uiView_);
      handleUiClick(ui.x, ui.y);
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
  // TODO: pełny restart nowej symulacji
  simulation_->start();
}

void SfmlSimulationVisualizer::stop() {
  // Stops visualization by closing the window.
  closeWindow();
}

void SfmlSimulationVisualizer::updateSceneViewport() {
  auto size = window_->getSize();
  const float w = static_cast<float>(size.x);
  const float h = static_cast<float>(size.y);

  const float uiHeightPx = uiBottomHeight_;
  const float panelH = std::min(uiHeightPx, h);
  const float vpH = std::max(0.f, (h - panelH) / h);

  // SCENA: tylko viewport (świat zostaje w układzie świata)
  sceneView_.reset(sf::FloatRect(0.f, 0.f, w, std::max(0.f, h - panelH)));
  sceneView_.setViewport(sf::FloatRect(0.f, 0.f, 1.f, vpH));

  // UI: ekranowe px
  uiView_.reset(sf::FloatRect(0.f, 0.f, w, panelH));
  uiView_.setViewport(sf::FloatRect(0.f, vpH, 1.f, 1.f - vpH));

  layoutUi();

  uiMargins_.left = 0.f;
  uiMargins_.right = statsPanelWidth_;
  uiMargins_.top = 0.f;
  uiMargins_.bottom = uiBottomHeight_;

  // zmiana viewportu wpływa na przelicznik px->świat (grubość linii)
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
  window_->setFramerateLimit(60);

  uiView_ = window_->getDefaultView();
  sceneView_ = uiView_;
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
  statsPanel_.setFont(&uiFont_);
  statsPanel_.setWidth(statsPanelWidth_);
  statsPanel_.setHeight(uiTopBarHeight_);
  updateSceneViewport();
  layoutUi();
}

void SfmlSimulationVisualizer::openSettings() {
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

  window_->clear(sf::Color(20, 22, 25));

  window_->setView(sceneView_);
  renderTo(*window_);
  drawStats(*window_);

  window_->setView(uiView_);
  drawUi(*window_);

  window_->display();
  if (settingsWindow_)
    settingsWindow_->tick();
}

void SfmlSimulationVisualizer::drawStats(sf::RenderTarget &rt) {
  StatsSnapshot snap;
  snap.simTimeSec = simulation_->getSimTime();
  snap.avgSpeed = simulation_->averageSpeed();

  const sf::Vector2u sz = window_->getSize();
  const float h = static_cast<float>(sz.y);
  const float panelH = h - uiBottomHeight_;

  uiTopBarHeight_ = panelH;
  statsPanel_.setHeight(panelH);
  statsPanel_.setWidth(statsPanelWidth_);
  uiMargins_.right = statsPanelWidth_;
  uiMargins_.bottom = uiBottomHeight_;

  statsPanel_.draw(rt, sz, snap);
}