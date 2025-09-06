#ifndef SFMLSTATSPANEL_H
#define SFMLSTATSPANEL_H
#include <SFML/Graphics.hpp>
#include <random>

struct StatsSnapshot {
  double simTimeSec = 0.0;
};

class SfmlStatsPanel {
public:
  SfmlStatsPanel();

  void setWidth(float w);
  void setFont(const sf::Font *font);
  void setTopBarHeight(float h);

  void draw(sf::RenderTarget &rt, const sf::Vector2u &windowSize,
            const StatsSnapshot &stats);

private:
  float width_{100.f};
  float topBarH_{0.f};
  const sf::Font *font_{nullptr};

  void drawPanel_(sf::RenderTarget &rt, const sf::Vector2u &sz) const;
  void drawTexts_(sf::RenderTarget &rt, const sf::Vector2u &sz,
                  const StatsSnapshot &stats) const;
};

#endif // SFMLSTATSPANEL_H
