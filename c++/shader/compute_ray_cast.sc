#include <bgfx_compute.sh>

BUFFER_RO(b_vertices, vec4, 0);
BUFFER_RO(b_indices, uint, 1);
BUFFER_RO(b_ray_origins, vec4, 2);
BUFFER_RO(b_ray_dirs, vec4, 3);
IMAGE2D_WR(b_results, rgba32f, 4);

uniform vec4 u_params;
#define num_indices u_params.x

bool intersectTriangle(vec3 orig, vec3 dir, vec3 v0, vec3 v1, vec3 v2, out float t, out float u, out float v)
{
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 h = cross(dir, e2);
    float a = dot(e1, h);

    if (abs(a) < 1e-6)
        return false;

    float f = 1.0 / a;
    vec3 s = orig - v0;
    u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(s, e1);
    v = f * dot(dir, q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    t = f * dot(e2, q);
    return t > 1e-6;
}

NUM_THREADS(256, 1, 1)
void main()
{
    uint ray_idx = gl_GlobalInvocationID.x;
    
    vec3 ray_origin = b_ray_origins[ray_idx].xyz;
    vec3 ray_dir = b_ray_dirs[ray_idx].xyz;
    
    float min_t = 3.402823466e+38; // FLT_MAX
    vec3 intersection_point = vec3(0.0, 0.0, 0.0);
    bool has_hit = false;

    uint num_triangles = uint(num_indices) / 3;
    for (uint tri_idx = 0; tri_idx < num_triangles; ++tri_idx)
    {
        uint i0 = b_indices[tri_idx * 3 + 0];
        uint i1 = b_indices[tri_idx * 3 + 1];
        uint i2 = b_indices[tri_idx * 3 + 2];

        vec3 v0 = b_vertices[i0].xyz;
        vec3 v1 = b_vertices[i1].xyz;
        vec3 v2 = b_vertices[i2].xyz;

        float t, u, v;
        if (intersectTriangle(ray_origin, ray_dir, v0, v1, v2, t, u, v))
        {
            if (t < min_t)
            {
                min_t = t;
                intersection_point = ray_origin + ray_dir * t;
                has_hit = true;
            }
        }
    }

    imageStore(b_results, ivec2(ray_idx, 0), vec4(intersection_point, has_hit ? 1.0 : 0.0));
}
