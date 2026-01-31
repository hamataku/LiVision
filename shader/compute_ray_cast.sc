#include <bgfx_compute.sh>

#define LOCAL_SIZE_X 64
#define LOCAL_SIZE_Y 16

BUFFER_RO(b_vertices, vec4, 0);
BUFFER_RO(b_ray_dirs, vec4, 1);
BUFFER_RO(b_positions, vec4, 2);
BUFFER_RO(b_mtx_invs, mat4, 3);
BUFFER_RO(b_mtx_randoms, mat4, 4);
BUFFER_RO(b_lidar_ranges, float, 5);
BUFFER_RO(b_bvh_nodes, vec4, 7);
BUFFER_RO(b_bvh_tri_indices, float, 8);

IMAGE2D_WR(b_results, rgba32f, 6);

uniform vec4 u_params;
uniform vec4 u_bvh_params;

#define num_indices u_params.x
#define num_rays u_params.y
#define num_lidars u_params.z
#define num_nodes u_bvh_params.x
#define num_tri_indices u_bvh_params.y

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

bool intersectAABB(vec3 orig, vec3 dir, vec3 bmin, vec3 bmax, out float tmin) {
    vec3 inv_dir = 1.0 / dir;
    vec3 t0 = (bmin - orig) * inv_dir;
    vec3 t1 = (bmax - orig) * inv_dir;
    vec3 tmin3 = min(t0, t1);
    vec3 tmax3 = max(t0, t1);
    float tminv = max(max(tmin3.x, tmin3.y), max(tmin3.z, 0.0));
    float tmaxv = min(min(tmax3.x, tmax3.y), tmax3.z);
    tmin = tminv;
    return tmaxv >= tminv;
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

    if (num_nodes <= 0.5 || num_tri_indices <= 0.5) {
        imageStore(b_results, ivec2(ray_idx, lidar_idx),
               vec4(0.0, 0.0, 0.0, 0.0));
        return;
    }

    float min_t = 3.0e+37;
    float max_range = b_lidar_ranges[lidar_idx];
    vec3 min_ray_dir = vec3(0.0, 0.0, 0.0);
    bool has_hit = false;

    uint stack[64];
    int sp = 0;
    stack[sp++] = 0u;

    while (sp > 0) {
        uint node_idx = stack[--sp];
        uint base = node_idx * 3u;
        vec4 nmin = b_bvh_nodes[base];
        vec4 nmax = b_bvh_nodes[base + 1u];
        vec4 nmeta = b_bvh_nodes[base + 2u];

        float node_tmin;
        if (!intersectAABB(ray_origin, ray_dir, nmin.xyz, nmax.xyz, node_tmin)) {
            continue;
        }
        if (node_tmin > min_t) {
            continue;
        }

        bool is_leaf = nmeta.z > 0.5;
        if (is_leaf) {
            uint first = uint(nmeta.x);
            uint count = uint(nmeta.y);
            for (uint i = 0u; i < count; ++i) {
                uint tri_idx = uint(b_bvh_tri_indices[first + i]);
                vec3 v0 = b_vertices[tri_idx * 3u].xyz;
                vec3 v1 = b_vertices[tri_idx * 3u + 1u].xyz;
                vec3 v2 = b_vertices[tri_idx * 3u + 2u].xyz;

                float t, u, v;
                if (intersectTriangle(ray_origin, ray_dir, v0, v1, v2, t, u, v)) {
                    if (t < min_t && t < max_range) {
                        min_t = t;
                        min_ray_dir = b_ray_dir.xyz;
                        has_hit = true;
                    }
                }
            }
        } else {
            uint left = uint(nmin.w);
            uint right = uint(nmax.w);

            float tmin_l = 0.0;
            float tmin_r = 0.0;
            bool hit_l = false;
            bool hit_r = false;

            uint left_base = left * 3u;
            uint right_base = right * 3u;
            vec4 lmin = b_bvh_nodes[left_base];
            vec4 lmax = b_bvh_nodes[left_base + 1u];
            vec4 rmin = b_bvh_nodes[right_base];
            vec4 rmax = b_bvh_nodes[right_base + 1u];

            hit_l = intersectAABB(ray_origin, ray_dir, lmin.xyz, lmax.xyz, tmin_l);
            hit_r = intersectAABB(ray_origin, ray_dir, rmin.xyz, rmax.xyz, tmin_r);

            if (hit_l && hit_r) {
                if (tmin_l < tmin_r) {
                    if (sp < 63) stack[sp++] = right;
                    if (sp < 63) stack[sp++] = left;
                } else {
                    if (sp < 63) stack[sp++] = left;
                    if (sp < 63) stack[sp++] = right;
                }
            } else if (hit_l) {
                if (sp < 63) stack[sp++] = left;
            } else if (hit_r) {
                if (sp < 63) stack[sp++] = right;
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