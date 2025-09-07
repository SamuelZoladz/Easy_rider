#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>

void SfmlSimulationVisualizer::rebuildGraphCache() {
  if (!graphProvider_)
    return;

  const auto data = graphProvider_();
  graphPositioning(data.nodePositions);

  edgesVA_ = std::make_unique<sf::VertexArray>(sf::Triangles);
  const float halfT = std::max(0.f, edgeThickness_) * 0.5f;

  for (const auto &e : data.edges) {
    const sf::Vector2f a = toScreen(data.nodePositions.at(e.first), view_);
    const sf::Vector2f b = toScreen(data.nodePositions.at(e.second), view_);

    // wektor kierunkowy
    sf::Vector2f d(b.x - a.x, b.y - a.y);
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 1e-4f) {
      // zbyt krótka krawędź — pomiń
      continue;
    }

    // wektor prostopadły znormalizowany (ekranowe piksele)
    sf::Vector2f n(-d.y / len * halfT, d.x / len * halfT);

    // cztery wierzchołki prostokąta
    const sf::Vector2f v0 = sf::Vector2f(a.x + n.x, a.y + n.y);
    const sf::Vector2f v1 = sf::Vector2f(b.x + n.x, b.y + n.y);
    const sf::Vector2f v2 = sf::Vector2f(b.x - n.x, b.y - n.y);
    const sf::Vector2f v3 = sf::Vector2f(a.x - n.x, a.y - n.y);

    // dwa trójkąty tworzące „grubą” linię
    edgesVA_->append(sf::Vertex(v0));
    edgesVA_->append(sf::Vertex(v1));
    edgesVA_->append(sf::Vertex(v2));

    edgesVA_->append(sf::Vertex(v2));
    edgesVA_->append(sf::Vertex(v3));
    edgesVA_->append(sf::Vertex(v0));
  }

  // === WĘZŁY: bez zmian ===
  nodesVA_ = std::make_unique<sf::VertexArray>(sf::Points);
  nodesVA_->clear();
  for (const auto &p : data.nodePositions) {
    nodesVA_->append(sf::Vertex(toScreen(p, view_)));
  }

  graphCacheDirty_ = false;
}

void SfmlSimulationVisualizer::graphPositioning(
    const std::vector<Vec2> &positions) {
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
