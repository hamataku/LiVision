#include "livision/object/primitives.hpp"

#include "livision/internal/mesh_buffer_manager.hpp"

namespace livision {

Box::Box(Params params) : ObjectBase(std::move(params)) {
  mesh_buf_ = internal::MeshBufferManager::AcquireShared(
      "primitive:box", []() {
        return std::make_shared<MeshBuffer>(
            std::vector<Vertex>{
                {-0.5F, 0.5F, 0.5F},
                {0.5F, 0.5F, 0.5F},
                {-0.5F, -0.5F, 0.5F},
                {0.5F, -0.5F, 0.5F},
                {-0.5F, 0.5F, -0.5F},
                {0.5F, 0.5F, -0.5F},
                {-0.5F, -0.5F, -0.5F},
                {0.5F, -0.5F, -0.5F},
            },
            std::vector<uint32_t>{
                0, 2, 1, 1, 2, 3, 4, 5, 6, 5, 7, 6, 0, 4, 2, 4, 6, 2,
                1, 3, 5, 5, 3, 7, 0, 1, 4, 4, 1, 5, 2, 6, 3, 6, 7, 3,
            });
      });
}

Cone::Cone(Params params) : ObjectBase(std::move(params)) {
  mesh_buf_ = internal::MeshBufferManager::AcquireShared(
      "primitive:cone", []() {
        // Implementation for creating cone vertex and index buffers
        constexpr int kPoly = 16;
        constexpr float kRadius = 0.5F;
        constexpr float kHeight = 1.0F;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // Apex at +Z/2, base center at -Z/2
        vertices.emplace_back(0.0F, 0.0F, kHeight * 0.5F);  // apex (idx 0)
        vertices.emplace_back(0.0F, 0.0F,
                              -kHeight * 0.5F);  // base center (idx 1)

        // base rim vertices start at index 2
        for (int i = 0; i < kPoly; ++i) {
          float theta = 2.0F * M_PI * static_cast<float>(i) / kPoly;
          vertices.emplace_back(kRadius * std::cos(theta),
                                kRadius * std::sin(theta), -kHeight * 0.5F);
        }

        const uint32_t apex_idx = 0;
        const uint32_t base_center_idx = 1;
        const uint32_t rim_start = 2;

        // side triangles (apex, rim_i, rim_{i+1})
        for (int i = 0; i < kPoly; ++i) {
          uint32_t i0 = rim_start + i;
          uint32_t i1 = rim_start + ((i + 1) % kPoly);
          indices.push_back(apex_idx);
          indices.push_back(i0);
          indices.push_back(i1);
        }

        // base triangles (base_center, rim_{i+1}, rim_i) so normal points -Z
        for (int i = 0; i < kPoly; ++i) {
          uint32_t i0 = rim_start + i;
          uint32_t i1 = rim_start + ((i + 1) % kPoly);
          indices.push_back(base_center_idx);
          indices.push_back(i1);
          indices.push_back(i0);
        }

        return std::make_shared<MeshBuffer>(vertices, indices);
      });
}

Cylinder::Cylinder(Params params) : ObjectBase(std::move(params)) {
  mesh_buf_ = internal::MeshBufferManager::AcquireShared(
      "primitive:cylinder", []() {
        constexpr int kPoly = 32;
        constexpr float kSize = 0.5F;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // make top cylinder_vertices
        for (int i = 0; i < kPoly; i++) {
          float theta = 2.0F * M_PI / kPoly * i;
          // top
          vertices.emplace_back(kSize * std::cos(theta), kSize * std::sin(theta),
                                kSize);
        }

        // make bottom cylinder_vertices
        for (int i = 0; i < kPoly; i++) {
          float theta = 2.0F * M_PI / kPoly * i;
          // bottom
          vertices.emplace_back(kSize * std::cos(theta), kSize * std::sin(theta),
                                -kSize);
        }

        auto append_cap = [&](uint32_t offset, bool flip_winding) {
          if (kPoly < 3) return;

          std::vector<uint32_t> strip;
          strip.reserve(kPoly);

          strip.push_back(offset + 1);
          strip.push_back(offset + 0);

          int left = 2;
          int right = kPoly - 1;
          bool take_left = true;

          while (left <= right) {
            if (take_left) {
              strip.push_back(offset + static_cast<uint32_t>(left));
              ++left;
            } else {
              strip.push_back(offset + static_cast<uint32_t>(right));
              --right;
            }
            take_left = !take_left;
          }

          for (size_t i = 0; i + 2 < strip.size(); ++i) {
            uint32_t a = strip[i];
            uint32_t b = strip[i + 2];
            uint32_t c = strip[i + 1];

            if ((i % 2) == 1) std::swap(a, b);
            if (flip_winding) std::swap(b, c);

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(c);
          }
        };

        // make top_indices (zigzag strip triangulation)
        append_cap(0, false);

        // make bottom_indices (reverse winding)
        append_cap(kPoly, true);

        // make side_indices
        for (int i = 0; i < kPoly; i++) {
          indices.push_back(i);
          indices.push_back(kPoly + i);
          indices.push_back((i + 1) % kPoly);

          indices.push_back((i + 1) % kPoly);
          indices.push_back(kPoly + i);
          indices.push_back(kPoly + ((i + 1) % kPoly));
        }

        return std::make_shared<MeshBuffer>(vertices, indices);
      });
}

Plane::Plane(Params params) : ObjectBase(std::move(params)) {
  mesh_buf_ = internal::MeshBufferManager::AcquireShared(
      "primitive:plane", []() {
        return std::make_shared<MeshBuffer>(
            std::vector<Vertex>{
                {-0.5F, 0.5F, 0.0F},   // top-left
                {0.5F, 0.5F, 0.0F},    // top-right
                {-0.5F, -0.5F, 0.0F},  // bottom-left
                {0.5F, -0.5F, 0.0F}    // bottom-right
            },
            std::vector<uint32_t>{0, 1, 2, 1, 3, 2, 0, 2, 1, 1, 2, 3});
      });
}

Sphere::Sphere(Params params) : ObjectBase(std::move(params)) {
  mesh_buf_ = internal::MeshBufferManager::AcquireShared(
      "primitive:sphere", []() {
        constexpr float kRadius = 0.5F;
        constexpr int kSubdiv = 2;  // 細分化回数（0: 正二十面体）

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        auto normalize_to_radius = [](const Vertex& v, float r) {
          float len = std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
          return Vertex{.x = v.x * r / len,
                        .y = v.y * r / len,
                        .z = v.z * r / len};
        };

        // ----- 正二十面体の初期頂点 -----
        const float t = (1.0F + std::sqrt(5.0F)) * 0.5F;

        std::vector<Vertex> base_vertices = {
            {-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
            {0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
            {t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1},
        };

        vertices.reserve(base_vertices.size());
        for (const auto& v : base_vertices) {
          vertices.emplace_back(normalize_to_radius(v, kRadius));
        }

        // ----- 正二十面体の三角形 -----
        indices = {0, 11, 5, 0, 5,  1,  0,  1,  7,  0,  7, 10, 0, 10, 11,
                   1, 5,  9, 5, 11, 4,  11, 10, 2,  10, 7, 6,  7, 1,  8,
                   3, 9,  4, 3, 4,  2,  3,  2,  6,  3,  6, 8,  3, 8,  9,
                   4, 9,  5, 2, 4,  11, 6,  2,  10, 8,  6, 7,  9, 8,  1};

        struct EdgeKey {
          uint32_t a, b;
          bool operator==(const EdgeKey& other) const {
            return a == other.a && b == other.b;
          }
        };

        struct EdgeHash {
          std::size_t operator()(const EdgeKey& k) const {
            return (static_cast<uint64_t>(k.a) << 32) | k.b;
          }
        };

        std::unordered_map<EdgeKey, uint32_t, EdgeHash> midpoint_cache;

        auto get_midpoint = [&](uint32_t i0, uint32_t i1) -> uint32_t {
          EdgeKey key{.a = std::min(i0, i1), .b = std::max(i0, i1)};
          auto it = midpoint_cache.find(key);
          if (it != midpoint_cache.end()) return it->second;

          Vertex mid = (vertices[i0] + vertices[i1]) * 0.5F;
          auto idx = static_cast<uint32_t>(vertices.size());
          vertices.emplace_back(normalize_to_radius(mid, kRadius));

          midpoint_cache[key] = idx;
          return idx;
        };

        for (int s = 0; s < kSubdiv; ++s) {
          std::vector<uint32_t> new_indices;
          midpoint_cache.clear();

          for (size_t i = 0; i < indices.size(); i += 3) {
            uint32_t i0 = indices[i + 0];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            uint32_t m0 = get_midpoint(i0, i1);
            uint32_t m1 = get_midpoint(i1, i2);
            uint32_t m2 = get_midpoint(i2, i0);

            new_indices.insert(new_indices.end(),
                               {i0, m0, m2, i1, m1, m0, i2, m2, m1, m0, m1, m2});
          }

          indices.swap(new_indices);
        }

        return std::make_shared<MeshBuffer>(vertices, indices);
      });
}

}  // namespace livision
