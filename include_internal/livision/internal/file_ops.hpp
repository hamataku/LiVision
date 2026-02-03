#pragma once

#include <fstream>
#include <iostream>

namespace livision::internal::file_ops {

inline std::streamoff StreamSize(std::istream& file) {
  std::istream::pos_type current_pos = file.tellg();
  if (current_pos == std::istream::pos_type(-1)) {
    return -1;
  }
  file.seekg(0, std::istream::end);
  std::istream::pos_type end_pos = file.tellg();
  file.seekg(current_pos);
  return end_pos - current_pos;
}

inline bool StreamReadString(std::istream& file, std::string& file_contents) {
  std::streamoff len = StreamSize(file);
  if (len == -1) {
    return false;
  }

  try {
    file_contents.resize(static_cast<std::string::size_type>(len));
    file.read(file_contents.data(), file_contents.length());
    return !file.fail();
  } catch (const std::exception&) {
    return false;
  }
}

inline bool ReadFile(const std::string& filename, std::string& file_contents) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    return false;
  }

  const bool success = StreamReadString(file, file_contents);

  file.close();

  return success;
}

}  // namespace livision::internal::file_ops
