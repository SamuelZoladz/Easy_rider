#include "Easy_rider/Parameters/Parameters.h"
#include "Easy_rider/Visualizers/SfmlSimulationVisualizer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>

namespace {
// Buttons in the bottom bar
enum class Btn : std::size_t { Restart = 0, Pause = 1, Stop = 2, Settings = 3 };

// Center a text inside a rectangle (with a small vertical tweak to match
// visuals)
void centerTextInRect(sf::Text &txt, const sf::FloatRect &r,
                      float yOffset = -2.f) {
  const auto bounds = txt.getLocalBounds();
  const float x = r.left + (r.width - bounds.width) * 0.5f - bounds.left;
  const float y =
      r.top + (r.height - bounds.height) * 0.5f - bounds.top + yOffset;
  txt.setPosition(x, y);
}
} // namespace

void SfmlSimulationVisualizer::initUiIfNeeded() {
  if (!window_)
    return;

  if (!uiFontLoaded_) {
    const std::string &fontPath = Parameters::fontPath();
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

  uiButtons_.clear();

  const auto sz = window_->getSize();
  const float w = static_cast<float>(sz.x);
  const float h = Parameters::uiBottomHeight();

  // Arrange 4 buttons centered horizontally within the panel.
  constexpr std::array<const char *, 4> labels{"Restart", "Pause", "Stop",
                                               "Settings"};
  const float totalW =
      4.f * Parameters::buttonWidth() + 3.f * Parameters::buttonGap();
  const float startX = std::max(Parameters::panelMargin(), (w - totalW) * 0.5f);
  const float y = (h - Parameters::buttonHeight()) * 0.5f;

  for (std::size_t i = 0; i < labels.size(); ++i) {
    const float x =
        startX + static_cast<float>(i) *
                     (Parameters::buttonWidth() + Parameters::buttonGap());
    uiButtons_.push_back(
        {{x, y, Parameters::buttonWidth(), Parameters::buttonHeight()},
         labels[i]});
  }
}

void SfmlSimulationVisualizer::drawUi(sf::RenderTarget &rt) {
  if (!window_)
    return;
  initUiIfNeeded();

  const auto sz = window_->getSize();
  const float w = static_cast<float>(sz.x);
  const float h = Parameters::uiBottomHeight();

  // Bottom panel background
  sf::RectangleShape panel({w, h});
  panel.setPosition(0.f, 0.f);
  panel.setFillColor(Parameters::argb(Parameters::panelBg()));
  panel.setOutlineThickness(1.f);
  panel.setOutlineColor(Parameters::argb(Parameters::panelOutline()));
  rt.draw(panel);

  // Buttons
  for (std::size_t i = 0; i < uiButtons_.size(); ++i) {
    const auto &b = uiButtons_[i];

    sf::RectangleShape rect({b.rect.width, b.rect.height});
    rect.setPosition({b.rect.left, b.rect.top});
    rect.setFillColor(Parameters::argb(Parameters::buttonBg()));
    rect.setOutlineThickness(2.f);
    rect.setOutlineColor(Parameters::argb(Parameters::buttonOutline()));
    rt.draw(rect);

    if (uiFontLoaded_) {
      sf::Text txt;
      txt.setFont(uiFont_);
      txt.setCharacterSize(Parameters::buttonTextSize());
      txt.setFillColor(Parameters::argb(Parameters::buttonTextColor()));

      // Dynamic label for Pause/Resume button (index 1)
      const bool isPause = (i == static_cast<std::size_t>(Btn::Pause));
      txt.setString(isPause ? (paused_ ? "Resume" : "Pause")
                            : uiButtons_[i].label);

      centerTextInRect(txt, b.rect);
      rt.draw(txt);
    }
  }
}

void SfmlSimulationVisualizer::handleUiClick(float mx, float my) {
  if (!window_)
    return;
  initUiIfNeeded();

  for (std::size_t i = 0; i < uiButtons_.size(); ++i) {
    if (!uiButtons_[i].rect.contains(mx, my))
      continue;

    switch (static_cast<Btn>(i)) {
    case Btn::Restart:
      restart();
      break;
    case Btn::Pause:
      paused_ ? resume() : pause();
      break;
    case Btn::Stop:
      stop();
      break;
    case Btn::Settings:
      openSettings();
      break;
    }
    break;
  }
}
