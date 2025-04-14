#include <bgfx_compute.sh>

#define CHUNK_TRIANGLES 64
#define LOCAL_SIZE 256

BUFFER_RO(b_vertices, vec4, 0);
BUFFER_RO(b_ray_dirs, vec4, 1);
IMAGE2D_WR(b_results, rgba32f, 2);

uniform vec4 u_params;
uniform mat4 u_mtx;
uniform mat4 u_mtx_inv;
uniform mat4 u_mtx_lidar;

#define num_indices u_params.x
#define num_rays u_params.y
#define max_range u_params.z

#define origin_x u_mtx[0][3]
#define origin_y u_mtx[1][3]
#define origin_z u_mtx[2][3]

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

NUM_THREADS(LOCAL_SIZE, 1, 1)
void main() {
    uint ray_idx = gl_GlobalInvocationID.x;

    if (ray_idx >= uint(num_rays)) {
        return;
    }

    vec3 ray_origin = vec3(origin_x, origin_y, origin_z);
    vec4 b_ray_dir = u_mtx_lidar * b_ray_dirs[ray_idx];
    vec3 ray_dir = (u_mtx_inv * b_ray_dir).xyz;

    float min_t = 3.0e+37;
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
        imageStore(b_results, ivec2(ray_idx, 0),
               vec4(min_ray_dir * min_t, 1.0));
    } else {
        imageStore(b_results, ivec2(ray_idx, 0),
               vec4(0.0, 0.0, 0.0, 0.0));
    }
}