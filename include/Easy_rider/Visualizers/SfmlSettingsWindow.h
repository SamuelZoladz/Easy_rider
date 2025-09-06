#ifndef SFMLSETTINGSWINDOW_H
#define SFMLSETTINGSWINDOW_H
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>

class SfmlSettingsWindow {
public:
  struct Callbacks {
    std::function<void()> onOpen;  // np. pause()
    std::function<void()> onClose; // np. resume()
  };

  explicit SfmlSettingsWindow(const sf::Font &uiFont, Callbacks cbs = {});
  ~SfmlSettingsWindow();

  void open();
  void close();
  bool isOpen() const;

  void tick();

private:
  void processEvents_();
  void render_();

  std::unique_ptr<sf::RenderWindow> win_;
  const sf::Font &font_;
  Callbacks cbs_;

  bool dragging_ = false; // czy aktualnie przeciągamy gałkę
  const float speedMin_ = 0.1f;
  const float speedMax_ = 10.f;
};
#endif // SFMLSETTINGSWINDOW_H
