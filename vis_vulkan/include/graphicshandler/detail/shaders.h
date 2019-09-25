#pragma once

#include <filesystem>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "shader_directories.h"

namespace gh::detail {

std::vector<char> readFile(std::filesystem::path filename);

std::vector<uint32_t> readIndices(std::filesystem::path filepath);

vk::UniqueShaderModule makeShaderModule(vk::Device& device, std::vector<char> code);

vk::UniqueShaderModule makeShaderModule(vk::Device& device,
                                        std::filesystem::path filepath);

namespace shader {

struct Holder {
  std::string name;

  vk::UniqueShaderModule vertex;
  vk::UniqueShaderModule fragment;

  std::vector<uint32_t> indexList;

  Holder(vk::UniqueShaderModule vertex, vk::UniqueShaderModule fragment,
         std::vector<uint32_t> indexList, std::string name = "default");
};

} // namespace shader

} // namespace gh::detail