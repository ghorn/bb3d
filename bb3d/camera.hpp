#pragma once

#include <glm/glm.hpp>

namespace bb3d {

class Camera {
 public:
  [[nodiscard]] glm::vec3 Eye() const;
  [[nodiscard]] glm::vec3 Center() const;
  [[nodiscard]] double Distance() const;
  void Rotate(float delta_x, float delta_y);
  void TranslateXy(float delta_x, float delta_y);
  void TranslateZ(float delta_x, float delta_y);
  void Scroll(float scroll_amount);
  void SetFocus(glm::vec3 new_focus){focus_position_ = new_focus;};
  void SetAzimuthDeg(float new_azimuth_deg){azimuth_deg_ = new_azimuth_deg;};
  void SetElevationDeg(float new_elevation_deg){elevation_deg_ = new_elevation_deg;};
  void SetDistance(float new_distance){distance_ = new_distance;};

 private:
  glm::vec3 focus_position_ = {0, 0, -2};
  float azimuth_deg_ = 60;
  float elevation_deg_ = 30;
  float distance_ = 10;
};

};  // namespace bb3d
