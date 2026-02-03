#pragma once

#include <Eigen/Core>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "livision/Vertex.hpp"

// Vec3用のハッシュ関数
namespace std {
template <>
struct hash<livision::Vertex> {
  size_t operator()(const livision::Vertex& v) const {
    return hash<float>()(v.x) ^ hash<float>()(v.y) ^ hash<float>()(v.z);
  }
};
}  // namespace std

namespace livision::stl_parser {

// 頂点バッファとインデックスバッファを構築する関数
inline void ParseSTLFile(const std::string& filename,
                         std::vector<livision::Vertex>& vertices,
                         std::vector<uint32_t>& indices) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    std::exit(EXIT_FAILURE);
  }

  file.seekg(80);  // ヘッダー（80バイト）をスキップ

  uint32_t num_triangles;
  file.read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));

  std::unordered_map<livision::Vertex, uint32_t> unique_vertices;
  vertices.clear();
  indices.clear();

  for (uint32_t i = 0; i < num_triangles; i++) {
    float normal[3];  // 法線
    file.read(reinterpret_cast<char*>(normal), sizeof(normal));

    for (int j = 0; j < 3; j++) {
      livision::Vertex triangle;
      file.read(reinterpret_cast<char*>(&triangle), sizeof(float) * 3);
      // 頂点が既に登録されているか確認
      auto it = unique_vertices.find(triangle);
      if (it == unique_vertices.end()) {
        auto index = static_cast<uint32_t>(vertices.size());
        unique_vertices[triangle] = index;
        vertices.push_back(triangle);
        indices.push_back(index);
      } else {
        indices.push_back(it->second);
      }
    }

    uint16_t attribute_byte_count;
    file.read(
        reinterpret_cast<char*>(&attribute_byte_count),
        sizeof(attribute_byte_count));  // 2バイトのアトリビュートをスキップ
  }

  std::cout << "[LiVision] STL: " << filename << ", " << indices.size() / 3
            << " meshes" << std::endl;
}
}  // namespace livision::stl_parser
