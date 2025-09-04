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

  // wywołuj raz na klatkę (obsługa eventów + render)
  void tick();

private:
  void processEvents_();
  void render_();

  std::unique_ptr<sf::RenderWindow> win_;
  const sf::Font &font_;
  Callbacks cbs_;
};
#endif // SFMLSETTINGSWINDOW_H
