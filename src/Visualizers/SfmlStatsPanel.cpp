#include "Easy_rider/Visualizers/SfmlStatsPanel.h"
#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>

SfmlStatsPanel::SfmlStatsPanel() { std::random_device rd; }

void SfmlSimulationVisualizer::drawStats(sf::RenderTarget &rt) {
  if (!window_)
    return;

  StatsSnapshot snap;
  snap.simTimeSec = simulation_->getSimTime();
  snap.avgSpeed = simulation_->averageSpeed();

  const sf::Vector2u sz = window_->getSize();
  const float h = static_cast<float>(sz.y);
  const float panelH = h - 100.f;

  uiTopBarHeight_ = panelH;
  statsPanel_.setTopBarHeight(panelH);
  statsPanel_.setWidth(100.f);

  statsPanel_.draw(rt, sz, snap);
}

void SfmlStatsPanel::setWidth(float w) { width_ = std::max(1.f, w); }
void SfmlStatsPanel::setFont(const sf::Font *font) { font_ = font; }
void SfmlStatsPanel::setTopBarHeight(float h) { topBarH_ = std::max(0.f, h); }

void SfmlStatsPanel::draw(sf::RenderTarget &rt, const sf::Vector2u &windowSize,
                          const StatsSnapshot &stats) {
  drawPanel_(rt, windowSize);
  drawTexts_(rt, windowSize, stats);
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

void SfmlStatsPanel::drawTexts_(sf::RenderTarget &rt, const sf::Vector2u &sz,
                                const StatsSnapshot &stats) const {
  if (!font_)
    return;

  const float w = static_cast<float>(sz.x);
  const float x = std::max(0.f, w - width_) + 8.f;
  const float y0 = 8.f;
  const float gap = 6.f; // vertical gap used below

  sf::Text title;
  title.setFont(*font_);
  title.setCharacterSize(12);
  title.setFillColor(sf::Color(230, 230, 230));

  sf::Text val;
  val.setFont(*font_);
  val.setCharacterSize(18);
  val.setStyle(sf::Text::Bold);
  val.setFillColor(sf::Color(255, 255, 255));

  float y = y0;

  title.setString("Simulation time:");
  title.setPosition(x, y);
  rt.draw(title);

  std::ostringstream ossSim;
  ossSim.setf(std::ios::fixed);
  ossSim << std::setprecision(2) << stats.simTimeSec;
  val.setString(ossSim.str() + " s");
  val.setPosition(x, y + 18.f + gap);
  rt.draw(val);

  // Advance below the first value, plus a little extra spacing before next
  // block
  y += (18.f + gap) + 18.f + 10.f;

  // --- Avg speed ---
  title.setString("Avg speed:");
  title.setPosition(x, y);
  rt.draw(title);

  std::ostringstream oss;
  oss.setf(std::ios::fixed);
  oss << std::setprecision(1) << stats.avgSpeed;

  val.setString(oss.str());
  val.setPosition(x, y + 18.f + gap);
  rt.draw(val);
}
