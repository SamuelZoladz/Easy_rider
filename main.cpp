#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>

int main() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Hello World");

  namespace fs = std::filesystem;
  auto exe_path = fs::current_path();

  sf::Font font;
  if (!font.loadFromFile((exe_path / "../arial.ttf").string())) {
    std::cerr << "Failed to load font from " << (exe_path / "../arial.ttf")
              << "\n";
    return EXIT_FAILURE;
  }

  sf::Text text;
  text.setFont(font);
  text.setString("Hello, World!");
  text.setCharacterSize(48);
  text.setFillColor(sf::Color::White);
  sf::FloatRect bounds = text.getLocalBounds();
  text.setOrigin(bounds.width / 2, bounds.height / 2);
  text.setPosition(800 / 2, 600 / 2);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear(sf::Color::Black);
    window.draw(text);
    window.display();
  }

  return 0;
}
