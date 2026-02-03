#pragma once
#include <Eigen/Core>
#include <vector>

namespace livision {

struct RainbowParams {
  Eigen::Vector3f direction{1.0F, 0.0F, 0.0F};
  float delta = 1.0F;

  RainbowParams() = default;
  RainbowParams(const Eigen::Vector3f& dir, float d)
      : direction(dir), delta(d) {}
};

struct Color {
  enum class ColorMode { Fixed = 0, Rainbow = 1 };

  ColorMode mode = ColorMode::Fixed;
  float base[4] = {0.0F, 0.0F, 0.0F, 1.0F};  // r, g, b, a
  RainbowParams rainbow;

  Color(float r, float g, float b, float a = 1.0F) : base{r, g, b, a} {};
  Color(float r, float g, float b, float a, const RainbowParams& rp)
      : base{r, g, b, a}, mode(ColorMode::Rainbow), rainbow(rp) {};
  Color(float r, float g, float b, const RainbowParams& rp)
      : base{r, g, b, 1.0F}, mode(ColorMode::Rainbow), rainbow(rp) {};
};

namespace color {
inline const Color white{1.0F, 1.0F, 1.0F};
inline const Color black{0.0F, 0.0F, 0.0F};

inline const Color gray{0.5F, 0.5F, 0.5F};
inline const Color light_gray{0.75F, 0.75F, 0.75F};
inline const Color off_white{0.9F, 0.9F, 0.9F};
inline const Color dark_gray{0.25F, 0.25F, 0.25F};

inline const Color red{1.0F, 0.0F, 0.0F};
inline const Color green{0.0F, 1.0F, 0.0F};
inline const Color blue{0.0F, 0.0F, 1.0F};
inline const Color yellow{1.0F, 1.0F, 0.0F};
inline const Color cyan{0.0F, 1.0F, 1.0F};
inline const Color magenta{1.0F, 0.0F, 1.0F};

inline const Color orange{0.95F, 0.45F, 0.10F};
inline const Color teal{0.10F, 0.65F, 0.65F};
inline const Color olive{0.65F, 0.70F, 0.20F};
inline const Color violet{0.65F, 0.40F, 0.90F};
inline const Color rose{0.95F, 0.35F, 0.45F};
inline const Color sand{0.90F, 0.80F, 0.55F};

inline const Color rainbow_z{
    1.0F, 0.0F, 0.0F, 1.0F,
    RainbowParams(Eigen::Vector3f(0.0F, 0.0F, 1.0F), 1.0F)};

inline std::vector<Color> color_palette{
    red, green, blue, yellow, cyan, magenta, orange, teal, olive, rose, sand};
}  // namespace color
}  // namespace livision