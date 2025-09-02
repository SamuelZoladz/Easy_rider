#include "Easy_rider/SfmlSimulationVisualizer.h"

#include <SFML/Graphics.hpp>
#include <cmath>

namespace {
static sf::Vector2f toScreen(const viz::Vec2 &w, const VisualizerView &v) {
  return {(w.x - v.originX) * v.scale, (w.y - v.originY) * v.scale};
}
} // namespace

SfmlSimulationVisualizer::SfmlSimulationVisualizer() = default;
SfmlSimulationVisualizer::~SfmlSimulationVisualizer() = default;

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
  window_ = std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height),
                                               title, sf::Style::Default);
  window_->setVerticalSyncEnabled(true);
}

void SfmlSimulationVisualizer::closeWindow() {
  if (window_) {
    window_->close();
    window_.reset();
  }
}

void SfmlSimulationVisualizer::processEvents() {
  if (!window_)
    return;
  sf::Event ev{};
  while (window_->pollEvent(ev)) {
    if (ev.type == sf::Event::Closed) {
      window_->close();
    } else if (ev.type == sf::Event::Resized) {
      resize(ev.size.width, ev.size.height);
    } else if (ev.type == sf::Event::KeyPressed &&
               ev.key.code == sf::Keyboard::Escape) {
      window_->close();
    }
  }
}

void SfmlSimulationVisualizer::resize(std::uint32_t width,
                                      std::uint32_t height) {
  if (!window_)
    return;
  sf::View view = window_->getView();
  view.setSize(static_cast<float>(width), static_cast<float>(height));
  view.setCenter(static_cast<float>(width) * 0.5f,
                 static_cast<float>(height) * 0.5f);
  window_->setView(view);
}

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

void SfmlSimulationVisualizer::renderFrame() {
  if (!window_)
    return;
  processEvents();
  window_->clear(sf::Color(20, 22, 25));
  renderTo(*window_);
  window_->display();
}

void SfmlSimulationVisualizer::run(double targetFps) {
  SimulationVisualizer::run(targetFps);
}
