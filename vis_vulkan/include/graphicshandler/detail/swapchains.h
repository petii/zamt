#pragma once

#include <vulkan/vulkan.hpp>

namespace gh::detail::swapchain {

struct Holder {
  vk::UniqueSwapchainKHR swapchain;
  vk::Format format;
  vk::Extent2D extent;

  Holder(vk::PhysicalDevice& physicalDevice, vk::Device& device, vk::SurfaceKHR& surface);

  void recreateSwapchain(int, int);

private:
  vk::PhysicalDevice& physicalDevice;
  vk::Device& device;
  vk::SurfaceKHR& surface;
};

vk::UniqueSwapchainKHR makeSwapchain(vk::PhysicalDevice&, vk::Device&, vk::SurfaceKHR&,
                                     vk::SwapchainKHR);

bool supported(vk::PhysicalDevice& device, vk::SurfaceKHR& surface,
               std::vector<uint32_t> queueFamilyIndices);

std::vector<vk::UniqueImage> getUniqueSwapchainImagesKHR(vk::Device& device,
                                                         vk::SwapchainKHR& swapchain);

template <vk::ImageAspectFlagBits Aspect>
vk::UniqueImageView createImageView(vk::Device& device, const vk::Image& image, vk::Format format)
{
  return device.createImageViewUnique(vk::ImageViewCreateInfo{
      vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D, format,
      vk::ComponentMapping(), vk::ImageSubresourceRange{Aspect, 0, 1, 0, 1}});
}

template <vk::ImageAspectFlagBits Aspect>
std::vector<vk::UniqueImageView> createImageViews(vk::Device& device,
                                                  const std::vector<vk::Image>& images, vk::Format format)
{
  std::vector<vk::UniqueImageView> results;
  results.reserve(images.size());
  for (auto& image : images) {
    results.emplace_back(createImageView<Aspect>(device, image, format));
  }
  return std::move(results);
}

} // namespace gh::detail::swapchain
