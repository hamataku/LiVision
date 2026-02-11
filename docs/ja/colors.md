# 色設定

LiVision では `livision::Color` で色を指定します。

## 固定色

`livision::color::*` を使います:

- `white`, `black`, `gray`, `light_gray`, `dark_gray`
- `red`, `green`, `blue`, `yellow`, `cyan`, `magenta`
- `orange`, `teal`, `olive`, `violet`, `rose`, `sand`

## レインボー色

- `rainbow_x`
- `rainbow_y`
- `rainbow_z`

## 非表示/透明

- `invisible`
- `transparent`

## 例

```cpp
auto box = livision::Box::Instance({
    .color = livision::color::rainbow_z,
    .wire_color = livision::color::black,
});
```
