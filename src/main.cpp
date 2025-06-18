/**
 * @file main.cpp
 * @brief Entry point for the Easy_rider application.
 */

#include "Easy_rider/HelloWorldApp.h"

/**
 * @brief Application entry point.
 *
 * Initializes and runs the HelloWorldApp.
 * @return EXIT_SUCCESS on normal exit, EXIT_FAILURE on error.
 */
int main() {
  HelloWorldApp app{800, 600, "Hello World"};
  return app.run();
}
