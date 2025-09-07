#include "Easy_rider/Visualizers/SfmlSettingsWindow.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "Easy_rider/Parameters/Parameters.h"

// Stała geometria suwaka (okno 520x380)
namespace {
constexpr float kPaddingX = 20.f;

// Teksty nad suwakiem
constexpr float kLabelY = 80.f;
constexpr float kValueY = 105.f;

// Suwak
constexpr float kTrackX = kPaddingX;
constexpr float kTrackY = 140.f;
constexpr float kTrackW = 480.f;
constexpr float kTrackH = 6.f;
constexpr float kKnobR = 10.f;

// pomocnicze mapowanie
inline float clampf(float v, float lo, float hi) {
  return std::max(lo, std::min(v, hi));
}
inline float toSliderT(float v, float vmin, float vmax) {
  v = clampf(v, vmin, vmax);
  const float a = std::log10(vmin);
  const float b = std::log10(vmax);
  const float x = std::log10(v);
  return (x - a) / (b - a);
}
inline float fromSliderT(float t, float vmin, float vmax) {
  t = clampf(t, 0.f, 1.f);
  const float a = std::log10(vmin);
  const float b = std::log10(vmax);
  const float x = a + t * (b - a);
  return std::pow(10.f, x);
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

  sf::VideoMode vm(520, 380);
  win_ = std::make_unique<sf::RenderWindow>(
      vm, "Settings", sf::Style::Titlebar | sf::Style::Close);
  win_->setFramerateLimit(60);
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

// --- prywatne ---
void SfmlSettingsWindow::processEvents_() {
  sf::Event ev{};
  while (win_->pollEvent(ev)) {
    if (ev.type == sf::Event::Closed) {
      close();
      return;
    }

    // Kliknięcie LPM: złap gałkę albo ustaw wartość klikając w tor
    if (ev.type == sf::Event::MouseButtonPressed &&
        ev.mouseButton.button == sf::Mouse::Left) {
      const sf::Vector2f mp =
          win_->mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});

      // oblicz aktualne centrum gałki wg skali log
      const float tNow =
          toSliderT(Parameters::simulationSpeed(), speedMin_, speedMax_);
      const float knobCx = kTrackX + tNow * kTrackW;
      const float knobCy = kTrackY + kTrackH * 0.5f;

      const sf::FloatRect knobBounds(knobCx - kKnobR, knobCy - kKnobR,
                                     2.f * kKnobR, 2.f * kKnobR);
      const sf::FloatRect trackBounds(kTrackX, kTrackY - 6.f, kTrackW,
                                      kTrackH + 12.f);

      if (knobBounds.contains(mp) || trackBounds.contains(mp)) {
        dragging_ = true;
        const float clampedX = clampf(mp.x, kTrackX, kTrackX + kTrackW);
        const float t = (clampedX - kTrackX) / kTrackW;
        Parameters::set_simulationSpeed(fromSliderT(t, speedMin_, speedMax_));
      }
    }
    if (ev.type == sf::Event::MouseButtonReleased &&
        ev.mouseButton.button == sf::Mouse::Left) {
      dragging_ = false;
    }

    if (ev.type == sf::Event::MouseMoved && dragging_) {
      const sf::Vector2f mp =
          win_->mapPixelToCoords({ev.mouseMove.x, ev.mouseMove.y});
      const float clampedX = clampf(mp.x, kTrackX, kTrackX + kTrackW);
      const float t = (clampedX - kTrackX) / kTrackW;
      Parameters::set_simulationSpeed(fromSliderT(t, speedMin_, speedMax_));
    }
  }
}

void SfmlSettingsWindow::render_() {
  win_->clear(sf::Color(32, 36, 40));

  // Tytuł
  {
    sf::Text title;
    title.setFont(font_);
    title.setCharacterSize(22);
    title.setString("Settings");
    title.setFillColor(sf::Color::White);
    title.setPosition(kPaddingX, 20.f);
    win_->draw(title);
  }

  // Label "Actual speed" + wartość nad suwakiem
  {
    sf::Text label;
    label.setFont(font_);
    label.setCharacterSize(18);
    label.setFillColor(sf::Color(220, 220, 220));
    label.setString("Actual speed");
    label.setPosition(kPaddingX, kLabelY);
    win_->draw(label);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << Parameters::simulationSpeed();
    sf::Text valueTxt;
    valueTxt.setFont(font_);
    valueTxt.setCharacterSize(18);
    valueTxt.setFillColor(sf::Color(180, 200, 255));
    valueTxt.setString(oss.str());
    valueTxt.setPosition(kPaddingX, kValueY);
    win_->draw(valueTxt);
  }

  // Suwak
  {
    const float t =
        toSliderT(Parameters::simulationSpeed(), speedMin_, speedMax_);

    // tor
    sf::RectangleShape track({kTrackW, kTrackH});
    track.setPosition(kTrackX, kTrackY);
    track.setFillColor(sf::Color(70, 76, 82));
    win_->draw(track);

    // wypełnienie do aktualnej wartości
    sf::RectangleShape fill({t * kTrackW, kTrackH});
    fill.setPosition(kTrackX, kTrackY);
    fill.setFillColor(sf::Color(110, 160, 255));
    win_->draw(fill);

    // gałka
    sf::CircleShape knob(kKnobR);
    knob.setOrigin(kKnobR, kKnobR);
    knob.setPosition(kTrackX + t * kTrackW, kTrackY + kTrackH * 0.5f);
    knob.setFillColor(sf::Color(230, 230, 230));
    win_->draw(knob);
  }

  win_->display();
}
