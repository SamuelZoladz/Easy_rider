/**
 * @file HelloWorldApp.h
 * @brief Declares the HelloWorldApp class, which encapsulates a simple SFML
 * application that displays "Hello, World!" centered in a window.
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <string>

/**
 * @class HelloWorldApp
 * @brief A simple SFML application that displays centered text.
 *
 * This class wraps the creation of an SFML RenderWindow, loading of a font,
 * setup of a text object, and the main loop handling events and rendering.
 */
class HelloWorldApp {
public:
  /**
   * @brief Constructs the application window.
   * @param width  The width of the window in pixels.
   * @param height The height of the window in pixels.
   * @param title  The title string to display in the window's title bar.
   *
   * Initializes the SFML RenderWindow and attempts to load resources.
   * If resource loading fails, the window is closed immediately.
   */
  HelloWorldApp(unsigned int width, unsigned int height,
                const std::string &title);

  /**
   * @brief Runs the main application loop.
   * @return EXIT_SUCCESS if the loop exits normally, EXIT_FAILURE otherwise.
   *
   * Processes events and renders the scene until the window is closed.
   */
  int run();

private:
  /**
   * @brief Loads fonts and sets up the text object.
   * @return True if all resources were loaded successfully; false on failure.
   *
   * Attempts to load "arial.ttf" from "../assets/fonts/" relative to
   * the current working directory, sets up the text string, size,
   * color, and origin for centering.
   */
  bool initResources();

  /**
   * @brief Processes all pending SFML window events.
   *
   * Handles the window closed event. Future event handling
   * (e.g., keyboard, mouse) can be added here.
   */
  void processEvents();

  /**
   * @brief Clears the window, draws the text, and displays the result.
   */
  void render();

private:
  sf::RenderWindow window_; ///< The main application window.
  sf::Font font_;           ///< The font used to render text.
  sf::Text text_;           ///< The text object displaying "Hello, World!".
};

/**
 * @brief Adds two integers.
 *
 * This function computes the sum of its two integer parameters.
 *
 * @param a First integer.
 * @param b Second integer.
 * @return Sum of a and b.
 */
int add(int a, int b);