#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <SFML/Graphics/Color.hpp>
#include <cstdint>
#include <string>

class Parameters {
public:
  Parameters() = delete;
  Parameters(const Parameters &) = delete;
  Parameters &operator=(const Parameters &) = delete;

  float &VehicleRadius() const;

  static void set_simulationSpeed(float s) { simulationSpeed_ = s; }
  static float simulationSpeed() { return simulationSpeed_; }

  static void set_fontPath(const std::string &path) { fontPath_ = path; }
  static const std::string &fontPath() { return fontPath_; }

  static void set_settingsWindowHeight(int h) { settingsWindowHeight_ = h; }
  static int settingsWindowHeight() { return settingsWindowHeight_; }

  static void set_settingsWindowWidth(int w) { settingsWindowWidth_ = w; }
  static int settingsWindowWidth() { return settingsWindowWidth_; }

  static void set_frameRateLimit(unsigned v) { frameRateLimit_ = v; }
  static unsigned frameRateLimit() { return frameRateLimit_; }

  static void set_mainWindowWidth(unsigned v) { mainWindowWidth_ = v; }
  static unsigned mainWindowWidth() { return mainWindowWidth_; }
  static void set_mainWindowHeight(unsigned v) { mainWindowHeight_ = v; }
  static unsigned mainWindowHeight() { return mainWindowHeight_; }

  static void set_uiBottomHeight(float v) { uiBottomHeight_ = v; }
  static float uiBottomHeight() { return uiBottomHeight_; }

  static void set_statsPanelWidth(float v) { statsPanelWidth_ = v; }
  static float statsPanelWidth() { return statsPanelWidth_; }

  static void set_buttonSize(float w, float h) {
    btnW_ = w;
    btnH_ = h;
  }
  static float buttonWidth() { return btnW_; }
  static float buttonHeight() { return btnH_; }

  static void set_buttonGap(float v) { btnGap_ = v; }
  static float buttonGap() { return btnGap_; }

  static void set_panelMargin(float v) { panelMargin_ = v; }
  static float panelMargin() { return panelMargin_; }

  static void set_buttonTextSize(unsigned v) { btnTextSize_ = v; }
  static unsigned buttonTextSize() { return btnTextSize_; }
  static void set_statsTitleSize(unsigned v) { statsTitleSize_ = v; }
  static unsigned statsTitleSize() { return statsTitleSize_; }
  static void set_statsValueSize(unsigned v) { statsValueSize_ = v; }
  static unsigned statsValueSize() { return statsValueSize_; }

  static void set_speedRange(float minV, float maxV) {
    speedMin_ = minV;
    speedMax_ = maxV;
  }
  static float speedMin() { return speedMin_; }
  static float speedMax() { return speedMax_; }

  static void set_baseEdgeThickness(float v) { baseEdgeThickness_ = v; }
  static float baseEdgeThickness() { return baseEdgeThickness_; }
  static void set_slowEdgeThicknessFactor(float v) {
    slowEdgeThicknessFacotr_ = v;
  }
  static float slowEdgeThicknessFactor() { return slowEdgeThicknessFacotr_; }
  static void set_fastEdgeThicknessFactor(float v) {
    fastEdgeThicknessFactor_ = v;
  }
  static float fastEdgeThicknessFactor() { return fastEdgeThicknessFactor_; }

  static void set_nodeRadius(float v) { nodeRadius_ = v; }
  static float nodeRadius() { return nodeRadius_; }
  static void set_vechicleRadius(float v) { vechicleRadius_ = v; }
  static float vechicleRadius() { return vechicleRadius_; }

  static void set_panelBg(uint32_t v) { panelBg_ = v; }
  static uint32_t panelBg() { return panelBg_; }
  static void set_panelOutline(uint32_t v) { panelOutline_ = v; }
  static uint32_t panelOutline() { return panelOutline_; }
  static void set_buttonBg(uint32_t v) { btnBg_ = v; }
  static uint32_t buttonBg() { return btnBg_; }
  static void set_buttonOutline(uint32_t v) { btnOutline_ = v; }
  static uint32_t buttonOutline() { return btnOutline_; }
  static void set_buttonTextColor(uint32_t v) { btnText_ = v; }
  static uint32_t buttonTextColor() { return btnText_; }
  static void set_backgroundColor(uint32_t v) { backgroundColor_ = v; }
  static uint32_t backgroundColor() { return backgroundColor_; }
  static void set_vechicleColor(uint32_t v) { vechicleColor_ = v; }
  static uint32_t vechicleColor() { return vechicleColor_; }

  static sf::Color argb(uint32_t c) {
    return sf::Color((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF,
                     (c >> 24) & 0xFF);
  }

  static void set_targetNodes(int v) { targetNodes_ = v; }
  static int targetNodes() { return targetNodes_; }
  static void set_minDistPx(int v) { minDistPx_ = v; }
  static int minDistPx() { return minDistPx_; }

  static void set_networkMargin(int v) { networkMargin_ = v; }
  static int networkMargin() { return networkMargin_; }

  static int networkMinX() { return networkMargin_; }
  static int networkMaxX() {
    return static_cast<int>(mainWindowWidth_) -
           static_cast<int>(statsPanelWidth_) - networkMargin_;
  }
  static int networkMinY() { return networkMargin_; }
  static int networkMaxY() {
    return static_cast<int>(mainWindowHeight_) -
           static_cast<int>(uiBottomHeight_) - networkMargin_;
  }

  static void set_motorwayThresholdRatio(double v) {
    motorwayThresholdRatio_ = v;
  }
  static double motorwayThresholdRatio() { return motorwayThresholdRatio_; }

  static void set_motorwayDefaultSpeed(int v) { motorwayDefaultSpeed_ = v; }
  static int motorwayDefaultSpeed() { return motorwayDefaultSpeed_; }

  static void set_motorwayCapacity(int v) { motorwayCapacity_ = v; }
  static int motorwayCapacity() { return motorwayCapacity_; }

  static void set_highwayDefaultSpeed(int v) { highwayDefaultSpeed_ = v; }
  static int highwayDefaultSpeed() { return highwayDefaultSpeed_; }

  static void set_highwayCapacity(int v) { highwayCapacity_ = v; }
  static int highwayCapacity() { return highwayCapacity_; }

  static void set_streetNumberOfNeighbors(int v) {
    streetNumberOfNeighbors_ = v;
  }
  static int streetNumberOfNeighbors() { return streetNumberOfNeighbors_; }

  static void set_streetDefaultSpeed(int v) { streetDefaultSpeed_ = v; }
  static int streetDefaultSpeed() { return streetDefaultSpeed_; }

  static void set_streetCapacity(int v) { streetCapacity_ = v; }
  static int streetCapacity() { return streetCapacity_; }

  static void set_isDijkstra(bool v) { isDijkstra_ = v; }
  static bool isDijkstra() { return isDijkstra_; }

private:
  inline static float simulationSpeed_ = 1.0f;
  inline static std::string fontPath_ = "assets/fonts/arial.ttf";
  inline static int settingsWindowHeight_ = 380;
  inline static int settingsWindowWidth_ = 520;

  inline static unsigned frameRateLimit_ = 60;
  inline static unsigned mainWindowWidth_ = 800;
  inline static unsigned mainWindowHeight_ =
      650; // Take into account the top bar size for closing the window.

  inline static float uiBottomHeight_ = 100.f;
  inline static float statsPanelWidth_ = 120.f;

  inline static float btnW_ = 180.f, btnH_ = 48.f, btnGap_ = 10.f,
                      panelMargin_ = 14.f;
  inline static unsigned btnTextSize_ = 18;
  inline static unsigned statsTitleSize_ = 12;
  inline static unsigned statsValueSize_ = 16;

  inline static float speedMin_ = 0.1f, speedMax_ = 10.f;

  inline static float baseEdgeThickness_ = 2.f, slowEdgeThicknessFacotr_ = 0.5f,
                      fastEdgeThicknessFactor_ = 2.f;
  inline static float nodeRadius_ = 7.f, vechicleRadius_ = 5.f;

  inline static uint32_t panelBg_ = 0xFF202428;
  inline static uint32_t panelOutline_ = 0xFF3C4044;
  inline static uint32_t btnBg_ = 0xFF383E44;
  inline static uint32_t btnOutline_ = 0xFF5A6068;
  inline static uint32_t btnText_ = 0xFFE6EBF0;
  inline static uint32_t backgroundColor_ = 0xFF141619;
  inline static u_int32_t vechicleColor_ = 0xFFFF0000;

  inline static int targetNodes_ = 30;
  inline static int minDistPx_ = 30;
  inline static int networkMargin_ = 50;

  inline static double motorwayThresholdRatio_ = 0.07;
  inline static int motorwayDefaultSpeed_ = 39;
  inline static int motorwayCapacity_ = 4;

  inline static int highwayDefaultSpeed_ = 25;
  inline static int highwayCapacity_ = 2;

  inline static int streetNumberOfNeighbors_ = 3;
  inline static int streetDefaultSpeed_ = 14;
  inline static int streetCapacity_ = 1;

  inline static bool isDijkstra_ = false;
};

#endif // PARAMETERS_H
