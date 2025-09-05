#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>

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

void SfmlSimulationVisualizer::layoutUi() {
  if (!window_)
    return;

  const auto sz = window_->getSize();
  const float w = static_cast<float>(sz.x);
  const float h = static_cast<float>(sz.y);
  const float panelH = std::min(100.f, h);

  const float margin = 14.f;
  const float btnW = 180.f, btnH = 48.f, gap = 10.f;

  const float totalW = 4.f * btnW + 3.f * gap;
  const float startX = std::max(margin, (w - totalW) * 0.5f);
  const float y = (panelH - btnH) * 0.5f;

  uiButtons_.clear();
  uiButtons_.push_back(
      {{startX + 0.f * (btnW + gap), y, btnW, btnH}, "Restart"});
  uiButtons_.push_back({{startX + 1.f * (btnW + gap), y, btnW, btnH}, "Pause"});
  uiButtons_.push_back({{startX + 2.f * (btnW + gap), y, btnW, btnH}, "Stop"});
  uiButtons_.push_back(
      {{startX + 3.f * (btnW + gap), y, btnW, btnH}, "Settings"});
}

void SfmlSimulationVisualizer::drawUi(sf::RenderTarget &rt) {
  if (!window_)
    return;
  initUiIfNeeded();

  const auto sz = window_->getSize();
  const float w = static_cast<float>(sz.x);
  const float h = static_cast<float>(sz.y);
  const float panelH = std::min(100.f, h);

  // panel w koordach UI: 0..w x 0..panelH
  sf::RectangleShape panel({w, panelH});
  panel.setPosition(0.f, 0.f);
  panel.setFillColor(sf::Color(32, 36, 40));
  panel.setOutlineThickness(1.f);
  panel.setOutlineColor(sf::Color(60, 64, 68));
  rt.draw(panel);

  for (std::size_t i = 0; i < uiButtons_.size(); ++i) {
    const auto &b = uiButtons_[i];

    sf::RectangleShape rect({b.rect.width, b.rect.height});
    rect.setPosition({b.rect.left, b.rect.top});
    rect.setFillColor(sf::Color(56, 62, 68));
    rect.setOutlineThickness(2.f);
    rect.setOutlineColor(sf::Color(90, 96, 104));
    rt.draw(rect);

    if (uiFontLoaded_) {
      sf::Text txt;
      txt.setFont(uiFont_);
      const bool isPauseBtn = (i == 1);
      txt.setString(isPauseBtn ? (paused_ ? "Resume" : "Pause")
                               : uiButtons_[i].label);
      txt.setCharacterSize(18);
      txt.setFillColor(sf::Color(230, 235, 240));

      auto bounds = txt.getLocalBounds();
      float tx =
          b.rect.left + (b.rect.width - bounds.width) * 0.5f - bounds.left;
      float ty = b.rect.top + (b.rect.height - bounds.height) * 0.5f -
                 bounds.top - 2.f;
      txt.setPosition(tx, ty);
      rt.draw(txt);
    }
  }
}

void SfmlSimulationVisualizer::handleUiClick(float mx, float my) {
  if (!window_)
    return;
  initUiIfNeeded();
  for (std::size_t i = 0; i < uiButtons_.size(); ++i) {
    if (uiButtons_[i].rect.contains(mx, my)) {
      if (i == 0)
        restart();
      else if (i == 1) {
        paused_ ? resume() : pause();
      } else if (i == 2) {
        stop();
      } else if (i == 3) {
        openSettings();
      }
      break;
    }
  }
}
