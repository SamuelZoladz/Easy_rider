#include "Easy_rider/Visualizers/SfmlStatsPanel.h"
#include "Easy_rider/Parameters/Parameters.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace {
// Small helper to format a floating value with fixed precision and optional
// suffix.
std::string formatFixed(float v, int precision, const char *suffix = nullptr) {
  std::ostringstream oss;
  oss.setf(std::ios::fixed);
  oss << std::setprecision(precision) << v;
  if (suffix && *suffix) {
    oss << ' ' << suffix;
  }
  return oss.str();
}
} // namespace

void SfmlStatsPanel::setWidth(float w) { width_ = std::max(1.f, w); }
void SfmlStatsPanel::setFont(const sf::Font *font) { font_ = font; }
void SfmlStatsPanel::setHeight(float h) { topBarH_ = std::max(0.f, h); }

void SfmlStatsPanel::draw(sf::RenderTarget &rt, const sf::Vector2u &windowSize,
                          const StatsSnapshot &stats) const {
  drawPanel_(rt, windowSize);
  drawTexts_(rt, windowSize, stats);
}

void SfmlStatsPanel::drawPanel_(sf::RenderTarget &rt,
                                const sf::Vector2u &sz) const {
  const float w = static_cast<float>(sz.x);
  const float x = std::max(0.f, w - width_);
  const float y = 0.f;
  const float hh = std::max(0.f, topBarH_);

  sf::RectangleShape bg({width_, hh});
  bg.setPosition(x, y);
  bg.setFillColor(Parameters::argb(Parameters::panelBg()));
  bg.setOutlineThickness(1.f);
  bg.setOutlineColor(Parameters::argb(Parameters::panelOutline()));
  rt.draw(bg);
}

void SfmlStatsPanel::drawTexts_(sf::RenderTarget &rt, const sf::Vector2u &sz,
                                const StatsSnapshot &stats) const {
  if (!font_)
    return;

  const float w = static_cast<float>(sz.x);
  const float sidePad = Parameters::panelMargin();
  const float topPad = Parameters::panelMargin();
  const float gap = Parameters::buttonGap();
  const float blockSpace = Parameters::panelMargin();

  const unsigned titleSize = Parameters::statsTitleSize();
  const unsigned valueSize = Parameters::statsValueSize();

  const float x0 = std::max(0.f, w - width_) + sidePad;
  float y = topPad;

  const sf::Color textColor = Parameters::argb(Parameters::buttonTextColor());

  sf::Text title;
  title.setFont(*font_);
  title.setCharacterSize(titleSize);
  title.setFillColor(textColor);

  sf::Text value;
  value.setFont(*font_);
  value.setCharacterSize(valueSize);
  value.setStyle(sf::Text::Bold);
  value.setFillColor(textColor);

  auto drawBlock = [&](const char *label, const std::string &valStr) {
    title.setString(label);
    title.setPosition(x0, y);
    rt.draw(title);

    value.setString(valStr);
    value.setPosition(x0, y + static_cast<float>(valueSize) + gap);
    rt.draw(value);

    y += (static_cast<float>(valueSize) + gap) + static_cast<float>(valueSize) +
         blockSpace;
  };

  drawBlock("Simulation time:", formatFixed(stats.simTimeSec, 2, "s"));
  drawBlock("Avg speed:", formatFixed(stats.avgSpeed, 1));
  drawBlock("Reroute saved:", formatFixed(stats.rerouteSavedTime, 1, "s"));
  drawBlock("Reroute counts:", formatFixed(stats.rerouteCounts, 0));
}
