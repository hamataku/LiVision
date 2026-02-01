#include <bgfx_compute.sh>

#define LOCAL_SIZE_X 64
#define LOCAL_SIZE_Y 16

BUFFER_RO(b_vertices, vec4, 0);
BUFFER_RO(b_ray_dirs, vec4, 1);
BUFFER_RO(b_positions, vec4, 2);
BUFFER_RO(b_mtx_invs, mat4, 3);
BUFFER_RO(b_mtx_randoms, mat4, 4);
BUFFER_RO(b_lidar_ranges, float, 5);

IMAGE2D_WR(b_results, rgba32f, 6);

uniform vec4 u_params;

#define num_indices u_params.x
#define num_rays u_params.y
#define num_lidars u_params.z

bool intersectTriangle(vec3 orig, vec3 dir, vec3 v0, vec3 v1, vec3 v2,
                       out float t, out float u, out float v) {
  vec3 e1 = v1 - v0;
  vec3 e2 = v2 - v0;
  vec3 h = cross(dir, e2);
  float a = dot(e1, h);

  if (abs(a) < 1e-6) return false;
  float f = 1.0 / a;
  vec3 s = orig - v0;
  u = f * dot(s, h);
  if (u < 0.0 || u > 1.0) return false;
  vec3 q = cross(s, e1);
  v = f * dot(dir, q);
  if (v < 0.0 || u + v > 1.0) return false;
  t = f * dot(e2, q);
  return t > 1e-6;
}

NUM_THREADS(LOCAL_SIZE_X, LOCAL_SIZE_Y, 1)
void main() {
    uint ray_idx = gl_GlobalInvocationID.x;
    uint lidar_idx = gl_GlobalInvocationID.y;

    if (ray_idx >= uint(num_rays) || lidar_idx >= uint(num_lidars)) {
        return;
    }

    vec4 b_position = b_positions[lidar_idx];

    vec3 ray_origin = vec3(b_position.x, b_position.y, b_position.z);
    vec4 b_ray_dir = b_mtx_randoms[lidar_idx] * b_ray_dirs[ray_idx];
    vec3 ray_dir = (b_mtx_invs[lidar_idx] * b_ray_dir).xyz;

    float min_t = 3.0e+37;
    float max_range = b_lidar_ranges[lidar_idx];
    vec3 min_ray_dir = vec3(0.0, 0.0, 0.0);
    bool has_hit = false;

    uint num_triangles = uint(num_indices);

    // 直接各三角形に対して交差判定を実施
    for (uint tri_idx = 0; tri_idx < num_triangles; tri_idx++) {
        vec3 v0 = b_vertices[tri_idx * 3].xyz;
        vec3 v1 = b_vertices[tri_idx * 3 + 1].xyz;
        vec3 v2 = b_vertices[tri_idx * 3 + 2].xyz;

        float t, u, v;
        if (intersectTriangle(ray_origin, ray_dir, v0, v1, v2, t, u, v)) {
            if (t < min_t && t < max_range) {
                min_t = t;
                min_ray_dir = b_ray_dir.xyz;
                has_hit = true;
            }
        }
    }

    if (has_hit) {
        imageStore(b_results, ivec2(ray_idx, lidar_idx),
               vec4(min_ray_dir * min_t, 1.0));
    } else {
        imageStore(b_results, ivec2(ray_idx, lidar_idx),
               vec4(0.0, 0.0, 0.0, 0.0));
    }
}