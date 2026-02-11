# Color Settings

LiVision uses `livision::Color` for base color and wireframe color.

## Fixed Colors

Use predefined colors from `livision::color::*`:

- `white`, `black`, `gray`, `light_gray`, `dark_gray`
- `red`, `green`, `blue`, `yellow`, `cyan`, `magenta`
- `orange`, `teal`, `olive`, `violet`, `rose`, `sand`

## Rainbow Colors

Rainbow mapping colors are available:

- `rainbow_x`
- `rainbow_y`
- `rainbow_z`

## Invisible/Transparent

- `invisible`
- `transparent`

## Example

```cpp
auto box = livision::Box::Instance({
    .color = livision::color::rainbow_z,
    .wire_color = livision::color::black,
});
```
