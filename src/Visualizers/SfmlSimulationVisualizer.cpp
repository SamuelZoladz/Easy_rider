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
  const float windowW = static_cast<float>(size.x);
  const float windowH = static_cast<float>(size.y);

  // wysokość dolnego paska UI (px)
  const float bottomUiHeightPx = std::min(uiBottomHeight_, windowH);
  // wysokość górnej części (scena + ewentualne overlaye) w px
  const float topContentHeightPx = std::max(0.f, windowH - bottomUiHeightPx);
  const float topContentHeightFrac =
      (windowH > 0.f) ? (topContentHeightPx / windowH) : 0.f;

  // SCENA: zajmuje górną część okna
  sceneView_.reset(sf::FloatRect(0.f, 0.f, windowW, topContentHeightPx));
  sceneView_.setViewport(sf::FloatRect(0.f, 0.f, 1.f, topContentHeightFrac));

  // UI (dolny pasek): zajmuje dolną część okna
  uiView_.reset(sf::FloatRect(0.f, 0.f, windowW, bottomUiHeightPx));
  uiView_.setViewport(sf::FloatRect(0.f, topContentHeightFrac, 1.f,
                                    1.f - topContentHeightFrac));

  layoutUi();

  // marginesy w pikselach ekranu
  uiMargins_.left = 0.f;
  uiMargins_.top = 0.f;
  uiMargins_.right = statsPanelWidth_;
  uiMargins_.bottom = uiBottomHeight_;

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
  initUiIfNeeded();
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
  const float windowH = static_cast<float>(sz.y);
  const float topContentHeightPx = std::max(0.f, windowH - uiBottomHeight_);

  uiTopBarHeight_ = topContentHeightPx;
  statsPanel_.setHeight(topContentHeightPx);
  statsPanel_.setWidth(statsPanelWidth_);
  uiMargins_.right = statsPanelWidth_;
  uiMargins_.bottom = uiBottomHeight_;

  statsPanel_.draw(rt, sz, snap);
}
