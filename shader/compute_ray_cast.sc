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
#define origin_x u_params.y
#define origin_y u_params.z
#define origin_z u_params.w

SHARED vec4 s_vertices[CHUNK_TRIANGLES * 3];

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

  vec3 ray_origin = vec3(origin_x, origin_y, origin_z);
  vec4 b_ray_dir = b_ray_dirs[ray_idx];
  vec3 ray_dir = (u_mtx_inv * u_mtx_lidar * b_ray_dir).xyz;

  float min_t = 3.0e+37;
  vec3 intersection_point_abs = vec3(0.0, 0.0, 0.0);
  bool has_hit = false;

  uint num_triangles = uint(num_indices);

  // チャンク単位で頂点データを共有メモリにロードして処理
  for (uint chunk_start = 0; chunk_start < num_triangles;
       chunk_start += CHUNK_TRIANGLES) {
    uint chunk_size = min(CHUNK_TRIANGLES, num_triangles - chunk_start);

    // 各スレッドが共有メモリに頂点データを読み込む
    for (uint i = gl_LocalInvocationID.x; i < chunk_size * 3; i += LOCAL_SIZE) {
      uint triangle_idx = chunk_start + (i / 3);  // 三角形のインデックス
      uint vertex_offset = i % 3;  // 三角形内の頂点オフセット
      uint global_idx =
          triangle_idx * 3 + vertex_offset;  // グローバルな頂点インデックス
      s_vertices[i] = b_vertices[global_idx];
    }

    barrier();  // 共有メモリの読み込み完了を待機

    // チャンク内の各三角形に対して交差判定を実施
    for (uint tri_local = 0; tri_local < chunk_size; tri_local++) {
      vec3 v0 = s_vertices[tri_local * 3].xyz;
      vec3 v1 = s_vertices[tri_local * 3 + 1].xyz;
      vec3 v2 = s_vertices[tri_local * 3 + 2].xyz;

      float t, u, v;
      if (intersectTriangle(ray_origin, ray_dir, v0, v1, v2, t, u, v)) {
        if (t < min_t) {
          min_t = t;
          intersection_point_abs = ray_origin + ray_dir.xyz * t;
          has_hit = true;
        }
      }
    }

    barrier();  // チャンク処理の完了を待機
  }

  vec4 intersection = u_mtx_inv * vec4(intersection_point_abs, 1.0) - vec4(ray_origin, 0.0);

  imageStore(b_results, ivec2(ray_idx, 0),
             vec4(intersection.xyz, has_hit ? 1.0 : 0.0));
}
