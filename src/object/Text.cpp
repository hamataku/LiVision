#include "livision/object/Text.hpp"

namespace livision {

Text::Text(Params params)
    : ObjectBase(static_cast<ObjectBase::Params>(params)),
      text_(std::move(params.text)),
      height_(params.height),
      font_(std::move(params.font)),
      facing_mode_(params.facing_mode),
      depth_mode_(params.depth_mode),
      align_(params.align) {}

void Text::OnDraw(Renderer& renderer) {
  renderer.SubmitText(text_, global_mtx_, params_.color, font_, height_,
                      facing_mode_, depth_mode_, align_);
}

Text* Text::SetText(const std::string& text) {
  text_ = text;
  return this;
}

Text* Text::SetHeight(float height) {
  height_ = height;
  return this;
}

Text* Text::SetFont(const std::string& font_path) {
  font_ = font_path;
  return this;
}

Text* Text::SetFacingMode(TextFacingMode mode) {
  facing_mode_ = mode;
  return this;
}

Text* Text::SetDepthMode(TextDepthMode mode) {
  depth_mode_ = mode;
  return this;
}

Text* Text::SetAlign(TextAlign align) {
  align_ = align;
  return this;
}

}  // namespace livision
