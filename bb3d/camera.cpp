#include "bb3d/camera.hpp"

#include <algorithm>  // min/max
#include <cmath>
#include <cstdio>

namespace bb3d {

glm::vec3 Camera::Eye() const {
  const float elevation = elevation_deg_ * static_cast<float>(M_PI) / 180.F;
  const float azimuth = azimuth_deg_ * static_cast<float>(M_PI) / 180.F;
  const float cos_elev = std::cos(elevation);
  const float x = distance_ * std::cos(azimuth) * cos_elev;
  const float y = distance_ * std::sin(azimuth) * cos_elev;
  const float z = -distance_ * std::sin(elevation);
  return Center() + glm::vec3(x, y, z);
}

glm::vec3 Camera::Center() const { return focus_position_; }

double Camera::Distance() const { return distance_; }

void Camera::Rotate(const float delta_x, const float delta_y) {
  azimuth_deg_ += delta_x;
  if (azimuth_deg_ > 180) {
    azimuth_deg_ -= 360;
  }
  if (azimuth_deg_ < -180) {
    azimuth_deg_ += 360;
  }

  elevation_deg_ += delta_y;
  elevation_deg_ = std::max(elevation_deg_, -89.F);
  elevation_deg_ = std::min(elevation_deg_, 89.F);
}

void Camera::TranslateXy(const float delta_x, const float delta_y) {
  const float sin_azi = std::sin(azimuth_deg_ * static_cast<float>(M_PI) / 180.F);
  const float cos_azi = std::cos(azimuth_deg_ * static_cast<float>(M_PI) / 180.F);
  const float scale = distance_ * 3e-3F;
  focus_position_.y += scale * (delta_x * cos_azi - delta_y * sin_azi);
  focus_position_.x -= scale * (delta_x * sin_azi + delta_y * cos_azi);
}

void Camera::TranslateZ(const float delta_x __attribute__((unused)), const float delta_y) {
  const float scale = distance_ * 3e-3F;
  focus_position_.z -= scale * delta_y;
}

void Camera::Scroll(const float scroll_amount) {
  if (scroll_amount > 0) {
    distance_ *= 1.15F;
    distance_ = std::min(distance_, 1e3F);
  } else if (scroll_amount < 0) {
    distance_ /= 1.15F;
    distance_ = std::max(distance_, 1e-3F);
  }
  if (scroll_amount != 1.F && scroll_amount != -1.F && scroll_amount != 0.F) {
    fprintf(stderr, "scroll expected to be in [-1, 0, 1] but it was %.1f\n",
            static_cast<double>(scroll_amount));
  }
}

};  // namespace bb3d
