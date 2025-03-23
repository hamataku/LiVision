#include <bgfx_compute.sh>

// Input buffers
BUFFER_RO(b_mesh, vec4, 0);     // Mesh vertices
BUFFER_RO(b_rays, vec4, 1);     // Ray directions
BUFFER_RO(b_bvh, vec4, 2);      // BVH nodes

// Output image
IMAGE2D_WR(img_output, rgba32f, 3);

// Uniforms
uniform vec4 u_params;
// u_params.x = triangle count
// u_params.yzw = ray origin (x,y,z)

// Constants
#define MAX_STACK 512
#define EPSILON 0.0000001

bool intersectAABB(vec3 origin, vec3 dir, vec3 boxMin, vec3 boxMax)
{
    vec3 invDir = 1.0 / dir;
    vec3 t0 = (boxMin - origin) * invDir;
    vec3 t1 = (boxMax - origin) * invDir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    float tenter = max(tmin.x, max(tmin.y, tmin.z));
    float texit = min(tmax.x, min(tmax.y, tmax.z));
    return tenter <= texit && texit > 0.0;
}

bool rayTriangleIntersection(vec3 rayOrigin, vec3 rayDir, vec3 v0, vec3 v1, vec3 v2, inout float t)
{
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(rayDir, edge2);
    float a = dot(edge1, h);

    // Ray parallel to triangle
    if (a > -EPSILON && a < EPSILON) return false;

    float f = 1.0 / a;
    vec3 s = rayOrigin - v0;
    float u = f * dot(s, h);

    // Intersection outside triangle
    if (u < 0.0 || u > 1.0) return false;

    vec3 q = cross(s, edge1);
    float v = f * dot(rayDir, q);

    // Intersection outside triangle
    if (v < 0.0 || u + v > 1.0) return false;

    // Calculate distance along ray
    t = f * dot(edge2, q);

    // Only accept intersections in front of the ray
    return t > EPSILON;
}

NUM_THREADS(256, 1, 1)
void main()
{
    // Get global thread ID
    uint ray_idx = gl_GlobalInvocationID.x;
    
    // Skip if outside bounds
    if (ray_idx >= imageSize(img_output).x)
        return;
        
    // Origin and ray direction
    vec3 origin = u_params.yzw;
    vec3 rayDir = b_rays[ray_idx].xyz;
    
    float closestT = 1.0e30; // Very large value
    bool hit = false;
    vec3 hitPoint = vec3(0.0, 0.0, 0.0);
    
    // BVH traversal stack (implemented as a fixed-size array)
    uint stack[MAX_STACK];
    int stackPtr = 0;
    
    // Start with root node
    stack[stackPtr++] = 0;
    
    while (stackPtr > 0)
    {
        // Pop node from stack
        uint nodeIdx = stack[--stackPtr];
        
        // Get BVH node data (stored as pairs of vec4)
        vec4 nodeData1 = b_bvh[nodeIdx * 3]; // min.xyz, max.x
        vec4 nodeData2 = b_bvh[nodeIdx * 3 + 1]; // max.yz, left_child, right_child
        vec4 nodeData3 = b_bvh[nodeIdx * 3 + 2]; // first_tri, tri_count, 0, 0
        
        // Extract AABB bounds
        vec3 boxMin = nodeData1.xyz;
        vec3 boxMax = vec3(nodeData1.w, nodeData2.xy);
        
        // Test ray against AABB
        if (!intersectAABB(origin, rayDir, boxMin, boxMax))
            continue;
            
        // Get children or triangles info
        uint triCount = uint(nodeData3.y);
        
        // Check if leaf node (triangle count > 0)
        if (triCount > 0)
        {
            // Leaf node: test triangles
            uint firstTri = uint(nodeData3.x);
            
            for (uint i = 0; i < triCount; i++)
            {
                uint triIdx = firstTri + i;
                vec3 v0 = b_mesh[triIdx * 3].xyz;
                vec3 v1 = b_mesh[triIdx * 3 + 1].xyz;
                vec3 v2 = b_mesh[triIdx * 3 + 2].xyz;
                
                float t;
                if (rayTriangleIntersection(origin, rayDir, v0, v1, v2, t))
                {
                    if (t < closestT)
                    {
                        closestT = t;
                        hit = true;
                        hitPoint = origin + rayDir * t;
                    }
                }
            }
        } else {
            // Internal node: push children to stack
            // Push right child first so left child is processed first
            uint rightChild = uint(nodeData2.z);
            uint leftChild = uint(nodeData2.w);
            
            // Check for stack overflow
            if (stackPtr < MAX_STACK - 1)
            {
                stack[stackPtr++] = rightChild;
                stack[stackPtr++] = leftChild;
            }
        }
    }
    
    // Write output
    vec4 result;
    if (hit) {
        result = vec4(hitPoint, 1.0); // w = 1.0 indicates a hit
    } else {
        result = vec4(0.0, 0.0, 0.0, 0.0); // w = 0.0 indicates no hit
    }
    
    imageStore(img_output, ivec2(ray_idx, 0), result);
}
