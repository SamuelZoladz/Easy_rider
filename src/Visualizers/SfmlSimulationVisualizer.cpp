#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <string>

SfmlSimulationVisualizer::SfmlSimulationVisualizer() = default;
SfmlSimulationVisualizer::~SfmlSimulationVisualizer() = default;

/**
 * @brief Lazy UI initialization — loads a font and lays out buttons.
 */
void SfmlSimulationVisualizer::initUiIfNeeded() {
  if (!window_)
    return;

  if (!uiFontLoaded_) {
    const std::string fontPath = "assets/fonts/arial.ttf";
    if (!uiFont_.loadFromFile(fontPath)) {
      throw std::runtime_error(
          "Font not found: " + fontPath +
          ".\nThe application requires this file to exist.");
    }
    uiFontLoaded_ = true;
  }

  if (uiButtons_.empty()) {
    layoutUi();
  }
}

/**
 * @brief Compute button rectangles based on the current window size.
 */
void SfmlSimulationVisualizer::layoutUi() {
  if (!window_)
    return;

  const auto sz = window_->getSize();
  const float w = static_cast<float>(sz.x);
  const float h = static_cast<float>(sz.y);

  const float panelW = std::min(panelWidth_, w);
  const float x0 = w - panelW;
  const float margin = 14.0f;
  const float btnH = 48.0f;
  const float gap = 10.0f;

  uiButtons_.clear();
  float y = margin;

  // Start
  uiButtons_.push_back(
      {sf::FloatRect{x0 + margin, y, panelW - 2.0f * margin, btnH}, "Restart"});
  y += btnH + gap;

  // Pause / Resume
  uiButtons_.push_back(
      {sf::FloatRect{x0 + margin, y, panelW - 2.0f * margin, btnH}, "Pause"});
  y += btnH + gap;

  // Stop
  uiButtons_.push_back(
      {sf::FloatRect{x0 + margin, y, panelW - 2.0f * margin, btnH}, "Stop"});
}

/**
 * @brief Draw the right-side panel and buttons (with dynamic labels).
 */
void SfmlSimulationVisualizer::drawUi(sf::RenderTarget &rt) {
  if (!window_)
    return;
  initUiIfNeeded();

  const auto sz = window_->getSize();
  const float w = static_cast<float>(sz.x);
  const float h = static_cast<float>(sz.y);
  const float panelW = std::min(panelWidth_, w);

  sf::RectangleShape panel(sf::Vector2f(panelW, h));
  panel.setPosition(sf::Vector2f(w - panelW, 0.0f));
  panel.setFillColor(sf::Color(32, 36, 40));
  panel.setOutlineThickness(1.0f);
  panel.setOutlineColor(sf::Color(60, 64, 68));
  rt.draw(panel);

  // Buttons
  for (std::size_t i = 0; i < uiButtons_.size(); ++i) {
    const auto &b = uiButtons_[i];

    sf::RectangleShape rect(sf::Vector2f(b.rect.width, b.rect.height));
    rect.setPosition(sf::Vector2f(b.rect.left, b.rect.top));
    rect.setFillColor(sf::Color(56, 62, 68));
    rect.setOutlineThickness(2.0f);
    rect.setOutlineColor(sf::Color(90, 96, 104));
    rt.draw(rect);

    if (uiFontLoaded_) {
      sf::Text txt;
      txt.setFont(uiFont_);

      const bool isPauseBtn = (i == 1);
      const std::string label =
          isPauseBtn ? (paused_ ? "Resume" : "Pause") : uiButtons_[i].label;

      txt.setString(label);
      txt.setCharacterSize(18);
      txt.setFillColor(sf::Color(230, 235, 240));

      const auto bounds = txt.getLocalBounds();
      const float tx =
          b.rect.left + (b.rect.width - bounds.width) * 0.5f - bounds.left;
      const float ty = b.rect.top + (b.rect.height - bounds.height) * 0.5f -
                       bounds.top - 2.0f;
      txt.setPosition(tx, ty);
      rt.draw(txt);
    }
  }
}

/**
 * @brief Handle a click in window coordinates; dispatch
 * Start/Pause-Resume/Stop.
 */
void SfmlSimulationVisualizer::handleUiClick(float mx, float my) {
  if (!window_)
    return;
  initUiIfNeeded();

  for (std::size_t i = 0; i < uiButtons_.size(); ++i) {
    if (uiButtons_[i].rect.contains(mx, my)) {
      // 0: Start, 1: Pause/Resume, 2: Stop
      if (i == 0) {
        restart();
      } else if (i == 1) {
        if (paused_)
          resume();
        else
          pause();
      } else if (i == 2) {
        stop();
      }
      break;
    }
  }
}

/**
 * @brief Handle window events (close, resize, mouse clicks, keyboard).
 *
 * - Left mouse click inside a button rect -> triggers button action.
 * - Space toggles Pause/Resume.
 * - Escape triggers Stop.
 */
void SfmlSimulationVisualizer::processEvents() {
  if (!window_)
    return;

  sf::Event ev{};
  while (window_->pollEvent(ev)) {
    if (ev.type == sf::Event::Closed) {
      window_->close();
    } else if (ev.type == sf::Event::Resized) {
      updateSceneViewport();
      layoutUi();
    } else if (ev.type == sf::Event::MouseButtonPressed &&
               ev.mouseButton.button == sf::Mouse::Left) {
      const float mx = static_cast<float>(ev.mouseButton.x);
      const float my = static_cast<float>(ev.mouseButton.y);
      handleUiClick(mx, my);
    } else if (ev.type == sf::Event::KeyPressed) {
      if (ev.key.code == sf::Keyboard::Space) {
        if (paused_)
          resume();
        else
          pause();
      } else if (ev.key.code == sf::Keyboard::Escape) {
        stop();
      }
    }
  }
}

bool SfmlSimulationVisualizer::isOpen() const {
  return window_ && window_->isOpen();
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
  window_->setVerticalSyncEnabled(true);
  window_->setFramerateLimit(60);

  uiView_ = window_->getDefaultView();
  sceneView_ = uiView_;
  updateSceneViewport();
  layoutUi();
}

void SfmlSimulationVisualizer::updateSceneViewport() {
  if (!window_)
    return;

  const auto sz = window_->getSize();
  const float w = static_cast<float>(sz.x);
  const float h = static_cast<float>(sz.y);

  const float panelW = std::min(panelWidth_, w);
  const float vpW = (w > 0.f) ? std::max(0.f, (w - panelW) / w) : 0.f;

  sceneView_.reset(sf::FloatRect(0.f, 0.f, w, h));
  sceneView_.setCenter(w * 0.5f, h * 0.5f);
  sceneView_.setViewport(sf::FloatRect(0.f, 0.f, vpW, 1.f));
  uiView_ = window_->getDefaultView();
}

void SfmlSimulationVisualizer::closeWindow() {
  if (window_) {
    window_->close();
    window_.reset();
  }
}

/// \brief Forces paused state (timeScale_ -> 0.0). Stores previous time scale.
void SfmlSimulationVisualizer::pause() {
  if (!paused_) {
    paused_ = true;
    prevTimeScale_ = timeScale_;
    timeScale_ = 0.0;
  }
}

/// \brief Resumes simulation restoring previous time scale (or 1.0).
void SfmlSimulationVisualizer::resume() {
  if (paused_) {
    paused_ = false;
    timeScale_ = (prevTimeScale_ > 0.0 ? prevTimeScale_ : 1.0);
  }
}

/// \brief Starts visualization: ensures window exists and resumes if paused.
void SfmlSimulationVisualizer::restart() {
  // TODO needs to restart completely new simulation
  simulation_->start();
}

/// \brief Stops visualization by closing the window.
void SfmlSimulationVisualizer::stop() { closeWindow(); }

void SfmlSimulationVisualizer::setGraphProvider(
    std::function<viz::GraphDrawData()> provider) {
  graphProvider_ = std::move(provider);
  graphCacheDirty_ = true;
}

void SfmlSimulationVisualizer::setVehicleProvider(
    std::function<std::vector<viz::Vec2>()> provider) {
  vehicleProvider_ = std::move(provider);
}

void SfmlSimulationVisualizer::onSimulationAttached() {
  graphCacheDirty_ = true;
}

void SfmlSimulationVisualizer::rebuildGraphCache() {
  if (!graphProvider_)
    return;

  const auto data = graphProvider_();

  edgesVA_ = std::make_unique<sf::VertexArray>(sf::Lines);
  edgesVA_->clear();
  for (const auto &e : data.edges) {
    const viz::Vec2 &a = data.nodePositions.at(e.first);
    const viz::Vec2 &b = data.nodePositions.at(e.second);
    edgesVA_->append(sf::Vertex(toScreen(a, view_)));
    edgesVA_->append(sf::Vertex(toScreen(b, view_)));
  }

  nodesVA_ = std::make_unique<sf::VertexArray>(sf::Points);
  nodesVA_->clear();
  for (const auto &p : data.nodePositions) {
    nodesVA_->append(sf::Vertex(toScreen(p, view_)));
  }

  graphCacheDirty_ = false;
}

void SfmlSimulationVisualizer::drawGraph(sf::RenderTarget &target) {
  if (graphCacheDirty_)
    rebuildGraphCache();
  if (edgesVA_)
    target.draw(*edgesVA_);
  if (nodesVA_) {
    sf::CircleShape node(nodeRadius_);
    node.setOrigin(nodeRadius_, nodeRadius_);
    for (std::size_t i = 0; i < nodesVA_->getVertexCount(); ++i) {
      node.setPosition(nodesVA_->operator[](i).position);
      target.draw(node);
    }
  }
}

void SfmlSimulationVisualizer::drawVehicles(sf::RenderTarget &target) {
  if (!vehicleProvider_)
    return;
  const auto vehicles = vehicleProvider_();
  sf::CircleShape dot(vehicleRadius_);
  dot.setOrigin(vehicleRadius_, vehicleRadius_);
  for (const auto &wpos : vehicles) {
    dot.setPosition(toScreen(wpos, view_));
    target.draw(dot);
  }
}

void SfmlSimulationVisualizer::renderTo(sf::RenderTarget &target) {
  drawGraph(target);
  drawVehicles(target);
}

/**
 * @brief Render single frame: process events, draw scene, draw UI, display.
 */
void SfmlSimulationVisualizer::renderFrame() {
  if (!window_)
    return;
  processEvents();

  window_->clear(sf::Color(20, 22, 25));
  window_->setView(sceneView_);
  renderTo(*window_);
  window_->setView(uiView_);
  drawUi(*window_);

  window_->display();
}

void SfmlSimulationVisualizer::run(double targetFps) {
  SimulationVisualizer::run(targetFps);
}
