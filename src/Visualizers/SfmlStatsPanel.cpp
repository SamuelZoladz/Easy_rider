#include "Easy_rider/Visualizers/SfmlStatsPanel.h"
#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>

SfmlStatsPanel::SfmlStatsPanel() {
  std::random_device rd;
  rng_ = std::mt19937{rd()};
  avgSpeed_ = 1.f; // wersja "temp" gdyby losowanie nie zaszło
}

void SfmlSimulationVisualizer::drawStats(sf::RenderTarget &rt) {
  // ta funkcja jest WYŁĄCZNIE od statystyk; żadnych przycisków/grafu
  if (!window_)
    return;

  const sf::Vector2u sz = window_->getSize();
  const float h = static_cast<float>(sz.y);
  const float panelH = h - 100.f;

  // przekazujemy geometrię do panelu
  uiTopBarHeight_ = panelH;
  statsPanel_.setTopBarHeight(panelH);
  statsPanel_.setWidth(100.f);

  // samodzielne rysowanie statystyk (y=0..panelH, prawa krawędź)
  statsPanel_.draw(rt, sz);
}

void SfmlStatsPanel::setWidth(float w) { width_ = std::max(1.f, w); }
void SfmlStatsPanel::setFont(const sf::Font *font) { font_ = font; }
void SfmlStatsPanel::setTopBarHeight(float h) { topBarH_ = std::max(0.f, h); }

void SfmlStatsPanel::maybeRefreshValue_() {
  if (refreshClock_.getElapsedTime() >= refreshEvery_) {
    avgSpeed_ = dist_(rng_);
    refreshClock_.restart();
  }
}

void SfmlStatsPanel::draw(sf::RenderTarget &rt,
                          const sf::Vector2u &windowSize) {
  maybeRefreshValue_();
  drawPanel_(rt, windowSize);
  drawTexts_(rt, windowSize);
}

void SfmlStatsPanel::drawPanel_(sf::RenderTarget &rt,
                                const sf::Vector2u &sz) const {
  const float w = static_cast<float>(sz.x);
  const float h = static_cast<float>(sz.y);

  const float x = std::max(0.f, w - width_);
  const float y = 0.f;
  const float hh = std::max(0.f, topBarH_);

  sf::RectangleShape bg({width_, hh});
  bg.setPosition(x, y);
  bg.setFillColor(sf::Color(32, 36, 40)); // tło
  bg.setOutlineThickness(1.f);
  bg.setOutlineColor(sf::Color(60, 64, 68));
  rt.draw(bg);
}

void SfmlStatsPanel::drawTexts_(sf::RenderTarget &rt,
                                const sf::Vector2u &sz) const {
  if (!font_)
    return;

  const float w = static_cast<float>(sz.x);
  const float x = std::max(0.f, w - width_) + 8.f;
  const float y = 8.f;

  sf::Text title;
  title.setFont(*font_);
  title.setCharacterSize(12);
  title.setFillColor(sf::Color(230, 230, 230));
  title.setString("Avg speed:");

  title.setPosition(x, y);
  rt.draw(title);

  std::ostringstream oss;
  oss.setf(std::ios::fixed);
  oss << std::setprecision(1) << avgSpeed_;

  sf::Text val;
  val.setFont(*font_);
  val.setCharacterSize(18);
  val.setStyle(sf::Text::Bold);
  val.setFillColor(sf::Color(255, 255, 255));

  val.setString(oss.str());
  val.setPosition(x, y + 18.f + 6.f);
  rt.draw(val);
}
