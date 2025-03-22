#include <array>
#include <fstream>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "FastLS/utils.hpp"

// Vec3用のハッシュ関数
namespace std {
template <>
struct hash<fastls::utils::Vec3Struct> {
  size_t operator()(const fastls::utils::Vec3Struct& v) const {
    return hash<float>()(v.x) ^ hash<float>()(v.y) ^ hash<float>()(v.z);
  }
};
}  // namespace std

namespace fastls {

// 頂点バッファとインデックスバッファを構築する関数
inline void ParseBinarySTL(const std::string& filename,
                           std::vector<utils::Vec3Struct>& vertices,
                           std::vector<uint32_t>& indices) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }

  file.seekg(80);  // ヘッダー（80バイト）をスキップ

  uint32_t num_triangles;
  file.read(reinterpret_cast<char*>(&num_triangles), sizeof(num_triangles));

  std::unordered_map<utils::Vec3Struct, uint32_t> unique_vertices;
  vertices.clear();
  indices.clear();

  for (uint32_t i = 0; i < num_triangles; i++) {
    float normal[3];  // 法線
    file.read(reinterpret_cast<char*>(normal), sizeof(normal));

    std::array<utils::Vec3Struct, 3> triangle;
    for (int j = 0; j < 3; j++) {
      file.read(reinterpret_cast<char*>(&triangle[j]),
                sizeof(utils::Vec3Struct));
      std::swap(triangle[j].x,
                triangle[j].z);  // STLとFastLSの座標系の違いを吸収

      // 頂点が既に登録されているか確認
      auto it = unique_vertices.find(triangle[j]);
      if (it == unique_vertices.end()) {
        auto index = static_cast<uint32_t>(vertices.size());
        unique_vertices[triangle[j]] = index;
        vertices.push_back(triangle[j]);
        indices.push_back(index);
        std::cout << triangle[j].x << " " << triangle[j].y << " "
                  << triangle[j].z << std::endl;
      } else {
        indices.push_back(it->second);
      }
    }

    uint16_t attribute_byte_count;
    file.read(
        reinterpret_cast<char*>(&attribute_byte_count),
        sizeof(attribute_byte_count));  // 2バイトのアトリビュートをスキップ
  }

  std::cout << "STL file: " << filename << std::endl;
  std::cout << "Vertices: " << vertices.size() << std::endl;
  std::cout << "Indices: " << indices.size() << std::endl;
}
}  // namespace fastls
