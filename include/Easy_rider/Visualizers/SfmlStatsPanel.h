#ifndef SFMLSTATSPANEL_H
#define SFMLSTATSPANEL_H
#include <SFML/Graphics.hpp>
#include <random>

class SfmlStatsPanel {
public:
  SfmlStatsPanel();

  void setWidth(float w);             // domyślnie 100 px
  void setFont(const sf::Font *font); // podaj istniejący font z UI
  void setTopBarHeight(float h); // wysokość belki z przyciskami (UI u góry)
  void draw(sf::RenderTarget &rt,
            const sf::Vector2u &windowSize); // rysuje panel po prawej

private:
  float width_{100.f};
  float topBarH_{0.f};
  const sf::Font *font_{nullptr};

  // "tymczasowa" średnia prędkość (losowo odświeżana)
  float avgSpeed_{1.f};
  std::mt19937 rng_;
  std::uniform_real_distribution<float> dist_{30.f, 70.f}; // km/h

  // czas do odświeżania wartości, żeby było widać że "żyje"
  sf::Clock refreshClock_;
  sf::Time refreshEvery_ = sf::milliseconds(600);

  void maybeRefreshValue_();
  void drawPanel_(sf::RenderTarget &rt, const sf::Vector2u &sz) const;
  void drawTexts_(sf::RenderTarget &rt, const sf::Vector2u &sz) const;
};
#endif // SFMLSTATSPANEL_H
