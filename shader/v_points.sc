$input a_position, i_data0
$output v_worldPos

#include <bgfx_shader.sh>

void main() {
    vec3 center = i_data0.xyz;
    float size = i_data0.w;

    vec3 localPos = a_position * size + center;
    vec4 worldPos = mul(u_model[0], vec4(localPos, 1.0));
    v_worldPos = worldPos.xyz;
    gl_Position = mul(u_modelViewProj, vec4(localPos, 1.0));
}
