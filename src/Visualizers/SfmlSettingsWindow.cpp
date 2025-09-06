#include "Easy_rider/Visualizers/SfmlSettingsWindow.h"

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
  win_->setVerticalSyncEnabled(true);
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
  }
}

void SfmlSettingsWindow::render_() {
  win_->clear(sf::Color(32, 36, 40));

  sf::Text title;
  title.setFont(font_);
  title.setCharacterSize(22);
  title.setString("Settings");
  title.setFillColor(sf::Color::White);
  title.setPosition(20.f, 20.f);
  win_->draw(title);

  win_->display();
}
