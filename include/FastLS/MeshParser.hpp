#pragma once

#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

// Vec3用のハッシュ関数
namespace std {
template <>
struct hash<glm::vec3> {
  size_t operator()(const glm::vec3& v) const {
    return hash<float>()(v.x) ^ hash<float>()(v.y) ^ hash<float>()(v.z);
  }
};
}  // namespace std

namespace fastls::mesh_parser {

// 頂点バッファとインデックスバッファを構築する関数
inline void ParseMeshFile(const std::string& filename,
                          std::vector<glm::vec3>& vertices,
                          std::vector<uint32_t>& indices) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }

  file.seekg(80);  // ヘッダー（80バイト）をスキップ

  uint32_t num_triangles;
  file.read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));

  std::unordered_map<glm::vec3, uint32_t> unique_vertices;
  vertices.clear();
  indices.clear();

  for (uint32_t i = 0; i < num_triangles; i++) {
    float normal[3];  // 法線
    file.read(reinterpret_cast<char*>(normal), sizeof(normal));

    for (int j = 0; j < 3; j++) {
      glm::vec3 triangle;
      file.read(reinterpret_cast<char*>(&triangle), sizeof(glm::vec3));
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

  std::cout << "file: " << filename << std::endl;
  std::cout << "Vertices: " << vertices.size() << std::endl;
  std::cout << "Meshes: " << indices.size() / 3 << std::endl;
}
}  // namespace fastls::mesh_parser
