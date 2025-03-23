#include "FastLS/SimLidar.hpp"

namespace fastls {

uint32_t SimLidar::CreateNode() {
  bvh_nodes_.emplace_back();
  return static_cast<uint32_t>(bvh_nodes_.size() - 1);
}

SimLidar::AABB SimLidar::ComputeSceneBounds() const {
  SimLidar::AABB bounds;
  for (const auto& vertex : mesh_vertices_) {
    bounds.Expand(glm::vec3(vertex));
  }
  return bounds;
}

void SimLidar::BuildBVH() {
  size_t triangle_count = mesh_vertices_.size() / 3;
  SimLidar::AABB scene_bounds = ComputeSceneBounds();

  bvh_nodes_.clear();
  bvh_nodes_.reserve(triangle_count * 2);

  uint32_t root_idx = CreateNode();
  SplitNode(root_idx, 0, triangle_count, scene_bounds, 0);
}

void SimLidar::SplitNode(uint32_t node_idx, uint32_t start, uint32_t count,
                         const SimLidar::AABB& bounds, int depth) {
  constexpr int kMaxDepth = 20;  // 最大深さ
  constexpr int kMinTris = 8;    // 葉ノードの最小三角形数

  BVHNode& node = bvh_nodes_[node_idx];
  node.bounds = bounds;

  if (count <= kMinTris || depth >= kMaxDepth) {
    // 葉ノードとして設定
    node.first_tri = start;
    node.tri_count = count;
    return;
  }

  // 空間の中央で分割
  int axis = depth % 3;  // x, y, z軸を順番に使用
  float split_pos = (bounds.min[axis] + bounds.max[axis]) * 0.5F;

  // 三角形を分割位置で仕分け
  uint32_t mid = start;
  for (uint32_t i = start; i < start + count; ++i) {
    glm::vec3 centroid = (glm::vec3(mesh_vertices_[i * 3]) +
                          glm::vec3(mesh_vertices_[i * 3 + 1]) +
                          glm::vec3(mesh_vertices_[i * 3 + 2])) /
                         3.0F;
    if (centroid[axis] < split_pos) {
      // 中心が分割面より手前なら左側に配置
      if (i != mid) {
        // 頂点の交換（3頂点セットで交換）
        for (int j = 0; j < 3; ++j) {
          std::swap(mesh_vertices_[i * 3 + j], mesh_vertices_[mid * 3 + j]);
        }
      }
      ++mid;
    }
  }

  uint32_t left_count = mid - start;
  if (left_count == 0 || left_count == count) {
    // 分割が失敗した場合は葉ノードとして処理
    node.first_tri = start;
    node.tri_count = count;
    return;
  }

  // 子ノードの作成
  uint32_t left_child = CreateNode();
  uint32_t right_child = CreateNode();

  node.left_child = left_child;
  node.right_child = right_child;
  node.tri_count = 0;  // 内部ノードは三角形を持たない

  // 左右の境界ボックスを計算
  SimLidar::AABB left_bounds;
  SimLidar::AABB right_bounds;
  for (uint32_t i = start; i < mid; ++i) {
    for (int j = 0; j < 3; ++j) {
      left_bounds.Expand(glm::vec3(mesh_vertices_[i * 3 + j]));
    }
  }
  for (uint32_t i = mid; i < start + count; ++i) {
    for (int j = 0; j < 3; ++j) {
      right_bounds.Expand(glm::vec3(mesh_vertices_[i * 3 + j]));
    }
  }

  // 子ノードを再帰的に分割
  SplitNode(left_child, start, left_count, left_bounds, depth + 1);
  SplitNode(right_child, mid, count - left_count, right_bounds, depth + 1);
}

void SimLidar::CreateBVHBuffer() {
  // BVHノードをGPUフォーマットに変換
  gpu_nodes_.reserve(bvh_nodes_.size() * 4);

  for (const auto& node : bvh_nodes_) {
    // 1つ目のvec4: min.xyz, max.x
    gpu_nodes_.emplace_back(node.bounds.min.x, node.bounds.min.y,
                            node.bounds.min.z, node.bounds.max.x);

    // 2つ目のvec4: max.yz, left_child, right_child
    gpu_nodes_.emplace_back(node.bounds.max.y, node.bounds.max.z,
                            static_cast<float>(node.left_child),
                            static_cast<float>(node.right_child));

    // 3つ目のvec4: first_tri, tri_count, 0, 0
    gpu_nodes_.emplace_back(static_cast<float>(node.first_tri),
                            static_cast<float>(node.tri_count), 0.0F, 0.0F);

    bvh_buffer_ = bgfx::createDynamicVertexBuffer(
        gpu_nodes_.size(), utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);

    const bgfx::Memory* mem =
        bgfx::makeRef(gpu_nodes_.data(), gpu_nodes_.size() * sizeof(glm::vec4));
    bgfx::update(bvh_buffer_, 0, mem);
  }
}

}  // namespace fastls
