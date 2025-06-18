/**
 * @file HelloWorldApp.cpp
 * @brief Implements the HelloWorldApp class for a simple SFML "Hello, World!"
 * demo.
 */

#include "Easy_rider/HelloWorldApp.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

HelloWorldApp::HelloWorldApp(unsigned int width, unsigned int height,
                             const std::string &title)
    : window_{sf::VideoMode(width, height), title} {
  if (!initResources()) {
    window_.close();
  }
}

bool HelloWorldApp::initResources() {
  // Determine path to the font file relative to the executable
  fs::path exe_path = fs::current_path();
  fs::path arial_path = exe_path / "../assets/fonts/arial.ttf";

  if (!font_.loadFromFile(arial_path.string())) {
    std::cerr << "Failed to load font from " << arial_path << "\n";
    return false;
  }

  // Configure the text object
  text_.setFont(font_);
  text_.setString("Hello, World!");
  text_.setCharacterSize(48);
  text_.setFillColor(sf::Color::White);

  // Center the text origin and position
  sf::FloatRect bounds = text_.getLocalBounds();
  text_.setOrigin(bounds.width / 2, bounds.height / 2);
  text_.setPosition(window_.getSize().x / 2.f, window_.getSize().y / 2.f);

  return true;
}

void HelloWorldApp::processEvents() {
  sf::Event event;
  while (window_.pollEvent(event)) {
    if (event.type == sf::Event::Closed)
      window_.close();
  }
}

void HelloWorldApp::render() {
  window_.clear(sf::Color::Black);
  window_.draw(text_);
  window_.display();
}

int HelloWorldApp::run() {
  while (window_.isOpen()) {
    processEvents();
    render();
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Adds two integers.
 *
 * Implementation of the add function: returns the arithmetic sum of a and b.
 *
 * @param a First integer.
 * @param b Second integer.
 * @return Sum of a and b.
 */
int add(int a, int b) { return a + b; }