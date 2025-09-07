#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <string>
#include <vector>

class Parameters {
public:
  Parameters() = delete;
  Parameters(const Parameters &) = delete;
  Parameters &operator=(const Parameters &) = delete;

  static void set_simulationSpeed(float s) { simulationSpeed_ = s; }
  static float simulationSpeed() { return simulationSpeed_; }

  static void set_fontPath(const std::string &path) { fontPath_ = path; }
  static const std::string &fontPath() { return fontPath_; }

private:
  inline static float simulationSpeed_ = 1.0f;
  inline static std::string fontPath_ = "assets/fonts/arial.ttf";
};

#endif // PARAMETERS_H
