#include "Easy_rider/Visualizers/SfmlSettingsWindow.h"
#include "Easy_rider/Parameters/Parameters.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>

// Fixed slider geometry for a 520x380 settings window.
namespace {
constexpr float kPaddingX = 20.f;

// Text above the slider
constexpr float kLabelY = 80.f;
constexpr float kValueY = 105.f;

constexpr float kTrackX = kPaddingX;
constexpr float kTrackY = 140.f;
constexpr float kTrackW = 480.f;
constexpr float kTrackH = 6.f;
constexpr float kKnobR = 10.f;

float clampf(float v, float lo, float hi) {
  return std::max(lo, std::min(v, hi));
}

float toSliderT(float v, float vmin, float vmax) {
  v = clampf(v, vmin, vmax);
  const float a = std::log10(vmin);
  const float b = std::log10(vmax);
  const float x = std::log10(v);
  return (x - a) / (b - a);
}

float fromSliderT(float t, float vmin, float vmax) {
  t = clampf(t, 0.f, 1.f);
  const float a = std::log10(vmin);
  const float b = std::log10(vmax);
  const float x = a + t * (b - a);
  return std::pow(10.f, x);
}

// Given a mouse X (in window coords), compute t in [0..1] along the track.
float sliderTFromMouseX(float mouseX) {
  const float clampedX = clampf(mouseX, kTrackX, kTrackX + kTrackW);
  return (clampedX - kTrackX) / kTrackW;
}
} // namespace

SfmlSettingsWindow::SfmlSettingsWindow(const sf::Font &uiFont, Callbacks cbs)
    : font_(uiFont), cbs_(std::move(cbs)) {}

SfmlSettingsWindow::~SfmlSettingsWindow() {
  if (win_ && win_->isOpen())
    win_->close();
}

void SfmlSettingsWindow::open() {
  if (isOpen())
    return;
  if (cbs_.onOpen)
    cbs_.onOpen();

  sf::VideoMode vm(Parameters::settingsWindowWidth(),
                   Parameters::settingsWindowHeight());
  win_ = std::make_unique<sf::RenderWindow>(
      vm, "Settings", sf::Style::Titlebar | sf::Style::Close);
  win_->setFramerateLimit(Parameters::frameRateLimit());
}

void SfmlSettingsWindow::close() {
  if (!win_)
    return;
  if (win_->isOpen())
    win_->close();
  win_.reset();
  if (cbs_.onClose)
    cbs_.onClose();
}

bool SfmlSettingsWindow::isOpen() const { return win_ && win_->isOpen(); }

void SfmlSettingsWindow::tick() {
  if (!isOpen())
    return;
  processEvents_();
  if (!isOpen())
    return;
  render_();
}

void SfmlSettingsWindow::processEvents_() {
  sf::Event ev{};
  while (win_->pollEvent(ev)) {
    if (ev.type == sf::Event::Closed) {
      close();
      return;
    }

    // Begin drag: left click on knob or track sets value immediately and grabs
    // the knob for dragging.
    if (ev.type == sf::Event::MouseButtonPressed &&
        ev.mouseButton.button == sf::Mouse::Left) {
      const sf::Vector2f mp =
          win_->mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});

      // Current knob center (log scale)
      const float tNow =
          toSliderT(Parameters::simulationSpeed(), Parameters::speedMin(),
                    Parameters::speedMax());
      const float knobCx = kTrackX + tNow * kTrackW;
      const float knobCy = kTrackY + kTrackH * 0.5f;

      const sf::FloatRect knobBounds(knobCx - kKnobR, knobCy - kKnobR,
                                     2.f * kKnobR, 2.f * kKnobR);
      const sf::FloatRect trackBounds(kTrackX, kTrackY - 6.f, kTrackW,
                                      kTrackH + 12.f);

      if (knobBounds.contains(mp) || trackBounds.contains(mp)) {
        dragging_ = true;
        const float t = sliderTFromMouseX(mp.x);
        Parameters::set_simulationSpeed(
            fromSliderT(t, Parameters::speedMin(), Parameters::speedMax()));
      }
    }

    // End drag
    if (ev.type == sf::Event::MouseButtonReleased &&
        ev.mouseButton.button == sf::Mouse::Left) {
      dragging_ = false;
    }

    // Dragging: update on mouse move
    if (ev.type == sf::Event::MouseMoved && dragging_) {
      const sf::Vector2f mp =
          win_->mapPixelToCoords({ev.mouseMove.x, ev.mouseMove.y});
      const float t = sliderTFromMouseX(mp.x);
      Parameters::set_simulationSpeed(
          fromSliderT(t, Parameters::speedMin(), Parameters::speedMax()));
    }
  }
}

void SfmlSettingsWindow::render_() {
  const sf::Color panelBg = Parameters::argb(Parameters::panelBg());
  const sf::Color textColor = Parameters::argb(Parameters::buttonTextColor());
  const sf::Color trackCol = Parameters::argb(Parameters::buttonBg());
  const sf::Color fillCol = Parameters::argb(Parameters::buttonOutline());
  const sf::Color knobCol = textColor;

  win_->clear(panelBg);

  // Title
  {
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(22);
    title.setString("Settings");
    title.setFillColor(textColor);
    title.setPosition(kPaddingX, 20.f);
    win_->draw(title);
  }

  // "Actual speed" label + numeric value above the slider
  {
    sf::Text label;
    label.setFont(font_);
    label.setCharacterSize(18);
    label.setFillColor(textColor);
    label.setString("Actual speed");
    label.setPosition(kPaddingX, kLabelY);
    win_->draw(label);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << Parameters::simulationSpeed();

    sf::Text valueTxt;
    valueTxt.setFont(font_);
    valueTxt.setCharacterSize(18);
    valueTxt.setFillColor(textColor);
    valueTxt.setString(oss.str());
    valueTxt.setPosition(kPaddingX, kValueY);
    win_->draw(valueTxt);
  }

  // Slider (track, filled portion, knob)
  {
    const float t = toSliderT(Parameters::simulationSpeed(),
                              Parameters::speedMin(), Parameters::speedMax());

    // Track
    sf::RectangleShape track({kTrackW, kTrackH});
    track.setPosition(kTrackX, kTrackY);
    track.setFillColor(trackCol);
    win_->draw(track);

    // Fill up to current value
    sf::RectangleShape fill({t * kTrackW, kTrackH});
    fill.setPosition(kTrackX, kTrackY);
    fill.setFillColor(fillCol);
    win_->draw(fill);

    // Knob
    sf::CircleShape knob(kKnobR);
    knob.setOrigin(kKnobR, kKnobR);
    knob.setPosition(kTrackX + t * kTrackW, kTrackY + kTrackH * 0.5f);
    knob.setFillColor(knobCol);
    win_->draw(knob);
  }

  win_->display();
}
