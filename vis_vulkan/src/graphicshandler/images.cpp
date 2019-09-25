#include "images.h"

namespace gh::detail::image {

Holder::Holder(vk::PhysicalDevice& physicalDevice, vk::Device& device,
               swapchain::Holder& swapchainHolder)
    : depth(makeDepthImage<vk::Format::eD32Sfloat>(physicalDevice, device,
                                                   swapchainHolder.extent)),
      device(device), physicalDevice(physicalDevice)
{
  swapchainImageViews = recreateSwapchainImages(swapchainHolder);
}

std::vector<vk::UniqueImageView>
Holder::recreateSwapchainImages(swapchain::Holder& swapchainHolder)
{
  // TODO: check if the images are leaked or not
  swapchainImages = device.getSwapchainImagesKHR(swapchainHolder.swapchain.get());
  auto newImageViews = swapchain::createImageViews<vk::ImageAspectFlagBits::eColor>(
      device, swapchainImages, swapchainHolder.format);
  return std::move(newImageViews);
}

} // namespace gh::detail::image