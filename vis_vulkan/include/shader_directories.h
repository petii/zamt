#pragma once

#include <string>
#include <filesystem>
#include <map>

using namespace std::string_literals;

namespace fs = std::filesystem;

enum class FileType {
  directory,
  vertexShader,
  fragmentShader,
  indicesList,
  unrecognized
};

inline std::string to_string(FileType f)
{
  switch (f) {
  case FileType::vertexShader:
    return "vert.spv"s;
  case FileType::fragmentShader:
    return "frag.spv"s;
  case FileType::indicesList:
    return "indices.txt"s;
  case FileType::directory:
    return "directory"s;
  default:
    // std::cerr << "File type unrecognized!" << std::endl;
    return ""s;
  }
}

inline FileType from_string(const std::string& str)
{
  if (to_string(FileType::vertexShader) == str) {
    return FileType::vertexShader;
  }
  if (to_string(FileType::fragmentShader) == str) {
    return FileType::fragmentShader;
  }
  if (to_string(FileType::indicesList) == str) {
    return FileType::indicesList;
  }
  return FileType::unrecognized;
}

using ShaderDirectoryContents = std::map<FileType, fs::path>;
