#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <string>

void SfmlSimulationVisualizer::graphPositioning(
    const std::vector<sf::Vector2f> &positions) {
  if (!window_) {
    view_.scale = 1.0f;
    view_.originX = 0.0f;
    view_.originY = 0.0f;
    view_.offsetX = uiMargins_.left + contentPaddingPx_;
    view_.offsetY = uiMargins_.top + contentPaddingPx_;
    return;
  }

  const sf::Vector2u sz = window_->getSize();
  const float totalW = static_cast<float>(sz.x);
  const float totalH = static_cast<float>(sz.y);

  const float availW =
      std::max(0.0f, totalW - (uiMargins_.left + uiMargins_.right));
  const float availH =
      std::max(0.0f, totalH - (uiMargins_.top + uiMargins_.bottom));

  // brak węzłów – domyślne wartości
  if (positions.empty() || availW <= 0.f || availH <= 0.f) {
    view_.scale = 1.0f;
    view_.originX = 0.0f;
    view_.originY = 0.0f;
    view_.offsetX = uiMargins_.left + contentPaddingPx_;
    view_.offsetY = uiMargins_.top + contentPaddingPx_;
    return;
  }

  float minX = std::numeric_limits<float>::infinity();
  float minY = std::numeric_limits<float>::infinity();
  float maxX = -std::numeric_limits<float>::infinity();
  float maxY = -std::numeric_limits<float>::infinity();
  for (const auto &pt : positions) {
    minX = std::min(minX, pt.x);
    minY = std::min(minY, pt.y);
    maxX = std::max(maxX, pt.x);
    maxY = std::max(maxY, pt.y);
  }

  const float padPx = contentPaddingPx_;
  const float fitW0 = std::max(1.0f, availW - 2.0f * padPx);
  const float fitH0 = std::max(1.0f, availH - 2.0f * padPx);

  const float worldW0 = std::max(1e-6f, maxX - minX);
  const float worldH0 = std::max(1e-6f, maxY - minY);

  // wstępna skala bez uwzględnienia grubości linii/promienia
  const float s0 = std::max(1e-6f, std::min(fitW0 / worldW0, fitH0 / worldH0));

  // dodaj zapas w pikselach (promień węzła / grubość krawędzi) przeliczony na
  // jednostki świata
  const float visualPadPx =
      std::max(nodeRadius_ * 2.0f, edgeThickness_ * 2.0f) + 2.0f;
  const float worldPad = visualPadPx / s0;

  const float worldW = worldW0 + 2.0f * worldPad;
  const float worldH = worldH0 + 2.0f * worldPad;

  // ostateczna skala
  const float fitW = std::max(1.0f, availW - 2.0f * padPx);
  const float fitH = std::max(1.0f, availH - 2.0f * padPx);
  float s = std::max(1e-6f, std::min(fitW / worldW, fitH / worldH));

  // origin przesunięty o worldPad żeby nic nie było ucięte
  const float originX = minX - worldPad;
  const float originY = minY - worldPad;

  // centracja w dostępnej przestrzeni
  const float usedW = worldW * s;
  const float usedH = worldH * s;
  const float offX =
      uiMargins_.left + padPx + 0.5f * std::max(0.0f, availW - usedW);
  const float offY =
      uiMargins_.top + padPx + 0.5f * std::max(0.0f, availH - usedH);

  view_.scale = s;
  view_.originX = originX;
  view_.originY = originY;
  view_.offsetX = offX;
  view_.offsetY = offY;
}

void SfmlSimulationVisualizer::rebuildGraphCache() {
  const auto data = makeGraphDrawData(simulation_->graph());
  graphPositioning(data.nodePositions);

  // piksel -> świat (dla grubości krawędzi stałej w px)
  const auto win = window_->getSize();
  const sf::FloatRect vp = sceneView_.getViewport();
  const float vpw = std::max(1.f, static_cast<float>(win.x) * vp.width);
  const float vph = std::max(1.f, static_cast<float>(win.y) * vp.height);
  const float sx = sceneView_.getSize().x / vpw;
  const float sy = sceneView_.getSize().y / vph;
  const float wpp = std::min(sx, sy);

  const float halfTWorld = std::max(0.f, edgeThickness_) * 0.5f * wpp;

  edgesVA_ = std::make_unique<sf::VertexArray>(sf::Triangles);
  edgesVA_->clear();

  for (const auto &e : data.edges) {
    const sf::Vector2f a = data.nodePositions.at(e.first);  // świat
    const sf::Vector2f b = data.nodePositions.at(e.second); // świat

    sf::Vector2f d(b.x - a.x, b.y - a.y);
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 1e-4f)
      continue;

    const sf::Vector2f n(-d.y / len * halfTWorld, d.x / len * halfTWorld);

    const sf::Vector2f v0 = sf::Vector2f(a.x + n.x, a.y + n.y);
    const sf::Vector2f v1 = sf::Vector2f(b.x + n.x, b.y + n.y);
    const sf::Vector2f v2 = sf::Vector2f(b.x - n.x, b.y - n.y);
    const sf::Vector2f v3 = sf::Vector2f(a.x - n.x, a.y - n.y);

    edgesVA_->append(sf::Vertex(v0));
    edgesVA_->append(sf::Vertex(v1));
    edgesVA_->append(sf::Vertex(v2));
    edgesVA_->append(sf::Vertex(v2));
    edgesVA_->append(sf::Vertex(v3));
    edgesVA_->append(sf::Vertex(v0));
  }

  nodesVA_.clear();
  nodesVA_.reserve(data.nodePositions.size());
  for (const sf::Vector2f &p : data.nodePositions) {
    nodesVA_.push_back(p);
  }

  graphCacheDirty_ = false;
}

void SfmlSimulationVisualizer::drawVehicles(sf::RenderTarget &target) {
  const auto vehicles = extractVehiclePositions(*simulation_);

  // piksel -> świat (dla stałego promienia pojazdu w px)
  const auto win = window_->getSize();
  const sf::FloatRect vp = sceneView_.getViewport();
  const float vpw = std::max(1.f, static_cast<float>(win.x) * vp.width);
  const float vph = std::max(1.f, static_cast<float>(win.y) * vp.height);
  const float sx = sceneView_.getSize().x / vpw;
  const float sy = sceneView_.getSize().y / vph;
  const float wpp = std::min(sx, sy);

  const float rW = std::max(0.f, vehicleRadius_) * wpp;

  sf::CircleShape dot(rW);
  dot.setOrigin(rW, rW);

  const auto old = target.getView();
  target.setView(sceneView_);
  for (const auto &wpos : vehicles) {
    dot.setPosition(wpos); // świat
    target.draw(dot);
  }
  target.setView(old);
}

void SfmlSimulationVisualizer::drawGraph(sf::RenderTarget &target) {
  if (graphCacheDirty_) {
    rebuildGraphCache();
  }

  const auto old = target.getView();
  target.setView(sceneView_);

  if (edgesVA_)
    target.draw(*edgesVA_);

  if (!nodesVA_.empty()) {
    const auto win = window_->getSize();
    const sf::FloatRect vp = sceneView_.getViewport();
    const float vpw = std::max(1.f, static_cast<float>(win.x) * vp.width);
    const float vph = std::max(1.f, static_cast<float>(win.y) * vp.height);
    const float sx = sceneView_.getSize().x / vpw;
    const float sy = sceneView_.getSize().y / vph;
    const float wpp = std::min(sx, sy);

    const float rW = std::max(0.f, nodeRadius_) * wpp;

    sf::CircleShape node(rW);
    node.setOrigin(rW, rW);
    for (const sf::Vector2f &wpos : nodesVA_) {
      node.setPosition(wpos);
      target.draw(node);
    }
  }

  target.setView(old);
}
