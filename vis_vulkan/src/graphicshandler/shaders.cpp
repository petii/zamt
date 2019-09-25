#include "shaders.h"

#include <fstream>
#include <iostream>

namespace gh::detail {

std::vector<char> readFile(std::filesystem::path filepath)
{
  // std::clog << __FUNCTION__ << ": current path: " << std::filesystem::current_path() << std::endl;
  auto filename = filepath.string();
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + filename);
  }
  auto fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  return buffer;
}

std::vector<uint32_t> readIndices(std::filesystem::path filepath)
{
  std::vector<uint32_t> result;
  // result.reserve(...);
  uint32_t currentlyRead;
  for (auto file = std::ifstream{filepath.string()}; file >> currentlyRead;) {
    result.emplace_back(currentlyRead);
  }
  return result;
}

vk::UniqueShaderModule makeShaderModule(vk::Device& device, std::vector<char> code)
{
  vk::ShaderModuleCreateInfo createInfo{vk::ShaderModuleCreateFlags(), code.size(),
                                        reinterpret_cast<uint32_t*>(code.data())};
  return device.createShaderModuleUnique(createInfo);
}

vk::UniqueShaderModule makeShaderModule(vk::Device& device,
                                        std::filesystem::path filepath)
{
  return makeShaderModule(device, readFile(filepath));
}

namespace shader {

Holder::Holder(vk::UniqueShaderModule vertex, vk::UniqueShaderModule fragment,
               std::vector<uint32_t> indexList, std::string name)
    : name(name), vertex(std::move(vertex)), fragment(std::move(fragment)),
      indexList(std::move(indexList))
{
}

} // namespace shader

} // namespace gh::detail