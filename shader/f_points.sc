$input v_worldPos

#include <bgfx_shader.sh>

uniform vec4 u_color;
uniform vec4 u_rainbow_params; // xyz = direction, w = delta
uniform vec4 u_color_mode;     // x = 0 fixed, 1 rainbow

vec3 rgb2hsv(vec3 c) {
    float maxc = max(c.r, max(c.g, c.b));
    float minc = min(c.r, min(c.g, c.b));
    float d = maxc - minc;
    float h = 0.0;
    if (d > 1e-6) {
        if (maxc == c.r) {
            h = (c.g - c.b) / d;
        } else if (maxc == c.g) {
            h = (c.b - c.r) / d + 2.0;
        } else {
            h = (c.r - c.g) / d + 4.0;
        }
        h = fract(h / 6.0);
        if (h < 0.0) h += 1.0;
    }
    float s = (maxc == 0.0) ? 0.0 : d / maxc;
    float v = maxc;
    return vec3(h, s, v);
}

vec3 hsv2rgb(vec3 c) {
    float h = c.x * 6.0;
    float s = c.y;
    float v = c.z;
    float i = floor(h);
    float f = h - i;
    float p = v * (1.0 - s);
    float q = v * (1.0 - s * f);
    float t = v * (1.0 - s * (1.0 - f));
    int ii = int(mod(i, 6.0));
    if (ii == 0) return vec3(v, t, p);
    if (ii == 1) return vec3(q, v, p);
    if (ii == 2) return vec3(p, v, t);
    if (ii == 3) return vec3(p, q, v);
    if (ii == 4) return vec3(t, p, v);
    return vec3(v, p, q);
}

void main() {
    vec4 outColor = u_color;

    if (int(u_color_mode.x) != 0) {
        vec3 base = u_color.rgb;
        vec3 hsv = rgb2hsv(base);
        vec3 dir = normalize(u_rainbow_params.xyz);
        float delta = u_rainbow_params.w;
        float hue_offset = fract(dot(dir, v_worldPos) * delta);
        hsv.x = fract(hsv.x + hue_offset);
        vec3 rgb = hsv2rgb(hsv);
        outColor = vec4(rgb, u_color.a);
    }

    gl_FragColor = outColor;
}
