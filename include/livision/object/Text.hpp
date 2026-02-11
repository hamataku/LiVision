#pragma once

#include <string>

#include "livision/ObjectBase.hpp"

namespace livision {

/**
 * @brief World-space text object.
 * @ingroup object
 */
class Text : public ObjectBase {
 public:
  struct Params : public ObjectBase::Params {
    Params() = default;
    std::string text;
    float height = 0.2F;
    std::string font;
    TextFacingMode facing_mode = TextFacingMode::Billboard;
    TextDepthMode depth_mode = TextDepthMode::DepthTest;
    TextAlign align = TextAlign::Left;
  };

  Text();
  explicit Text(Params params);

  void OnDraw(Renderer& renderer) final;

  Text& SetText(const std::string& text);
  Text& SetHeight(float height);
  Text& SetFont(const std::string& font_path);
  Text& SetFacingMode(TextFacingMode mode);
  Text& SetDepthMode(TextDepthMode mode);
  Text& SetAlign(TextAlign align);

 private:
  std::string text_;
  float height_ = 0.2F;
  std::string font_;
  TextFacingMode facing_mode_ = TextFacingMode::Billboard;
  TextDepthMode depth_mode_ = TextDepthMode::DepthTest;
  TextAlign align_ = TextAlign::Left;
};

}  // namespace livision
