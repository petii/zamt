#pragma once

#include <tuple>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "buffers.h"
#include "swapchains.h"

namespace gh::detail::image {

template <vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlagBits usage>
auto createImage(vk::Device& device, vk::Extent2D& extent)
{
  return device.createImageUnique(
      vk::ImageCreateInfo{vk::ImageCreateFlags(), vk::ImageType::e2D, format,
                          vk::Extent3D{extent.width, extent.height, 1}, 1, 1,
                          vk::SampleCountFlagBits::e1, tiling, usage});
}

template <vk::MemoryPropertyFlagBits property>
auto allocateAndBindMemory(vk::PhysicalDevice& physicalDevice, vk::Device& device,
                           vk::Image& image)
{
  // allocate
  auto memoryRequirements = device.getImageMemoryRequirements(image);
  auto memoryType =
      findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, property);
  auto memory = device.allocateMemoryUnique(
      vk::MemoryAllocateInfo{memoryRequirements.size, memoryType});
  // bind
  device.bindImageMemory(image, memory.get(), 0);
  return std::move(memory);
}

struct DepthHolder {
  vk::UniqueImage image;
  vk::UniqueDeviceMemory memory;
  vk::UniqueImageView view;
};

template <vk::Format DepthFormat = vk::Format::eD32Sfloat>
auto makeDepthImage(vk::PhysicalDevice& physicalDevice, vk::Device& device,
                    vk::Extent2D& extent)
{
  DepthHolder result;

  result.image =
      createImage<DepthFormat, vk::ImageTiling::eOptimal,
                  vk::ImageUsageFlagBits::eDepthStencilAttachment>(device, extent);
  result.memory = allocateAndBindMemory<vk::MemoryPropertyFlagBits::eDeviceLocal>(
      physicalDevice, device, result.image.get());
  result.view = device.createImageViewUnique(vk::ImageViewCreateInfo{
      vk::ImageViewCreateFlags(), result.image.get(), vk::ImageViewType::e2D, DepthFormat,
      vk::ComponentMapping(),
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}});

  return std::move(result);
}

struct Holder {
  std::vector<vk::Image> swapchainImages;
  std::vector<vk::UniqueImageView> swapchainImageViews;

  DepthHolder depth;

  Holder(vk::PhysicalDevice& physicalDevice, vk::Device& device,
         swapchain::Holder& swapchainHolder);

  std::vector<vk::UniqueImageView> recreateSwapchainImages(swapchain::Holder&);

private:
  vk::Device& device;
  vk::PhysicalDevice& physicalDevice;
};

} // namespace gh::detail::image
