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

private:
  inline static float simulationSpeed_ = 1.0f;
};

#endif // PARAMETERS_H
