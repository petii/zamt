#include "swapchains.h"

#include <algorithm>
#include <iostream>

#include "common.h"

constexpr auto uint32max = std::numeric_limits<int32_t>::max();

namespace gh::detail::swapchain {

struct SwapchainSupport {

  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;

  SwapchainSupport(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface)
      : capabilities(physicalDevice.getSurfaceCapabilitiesKHR(surface)),
        formats(physicalDevice.getSurfaceFormatsKHR(surface)),
        presentModes(physicalDevice.getSurfacePresentModesKHR(surface))
  {
    static bool firstTime = true;
    if (firstTime) {
      std::clog << "Swapchain info:" << std::endl
                << "  Capabilites: <TODO>" << std::endl
                << "  Formats (" << formats.size() << "): " << std::endl
                << "  Present modes (" << presentModes.size() << "): " << std::endl;
      firstTime = false;
    }
  }

  // TODO: handle formats
  vk::SurfaceFormatKHR chooseFormat() { return formats[0]; }

  vk::PresentModeKHR choosePresentMode(vk::PresentModeKHR hint = vk::PresentModeKHR::eFifo)
  {
    auto hinted = std::find(presentModes.begin(), presentModes.end(), hint);
    if (hinted != presentModes.end())
      return *hinted;

    auto tripleBuffering =
        std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eMailbox);
    if (tripleBuffering != presentModes.end())
      return *tripleBuffering;
    auto doubleBuffering =
        std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eFifo);
    if (doubleBuffering != presentModes.end())
      return *doubleBuffering;
    // TODO: check single buffer aka eImmediate
    return vk::PresentModeKHR::eImmediate;
  }

  vk::Extent2D chooseExtent(uint32_t width = uint32max, uint32_t height = uint32max)
  {
    if (capabilities.currentExtent.width != uint32max &&
        capabilities.currentExtent.height != uint32max) {
      return capabilities.currentExtent;
    }
    return {clamp(capabilities.minImageExtent.width, capabilities.maxImageExtent.width,
                  width),
            clamp(capabilities.minImageExtent.height, capabilities.maxImageExtent.height,
                  height)};
  }

  uint32_t getImageCount()
  {
    uint32_t images = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && images > capabilities.maxImageCount)
      return capabilities.maxImageCount;
    return images;
  }
};

vk::UniqueSwapchainKHR makeSwapchain(vk::PhysicalDevice& physicalDevice,
                                     vk::Device& device, vk::SurfaceKHR& surface,
                                     vk::SwapchainKHR oldSwapchain = vk::SwapchainKHR())
{
  SwapchainSupport support(physicalDevice, surface);
  auto format = support.chooseFormat();
  vk::SwapchainCreateInfoKHR createInfo{
      vk::SwapchainCreateFlagsKHR(), surface, support.getImageCount(), format.format,
      format.colorSpace, support.chooseExtent(), 1,
      vk::ImageUsageFlagBits::eColorAttachment,
      // TODO: different queue families
      vk::SharingMode::eExclusive, 0, nullptr, support.capabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque, support.choosePresentMode(), true,
      oldSwapchain};
  return device.createSwapchainKHRUnique(createInfo);
}

bool supported(vk::PhysicalDevice& device, vk::SurfaceKHR& surface,
               std::vector<uint32_t> queueFamilyIndices)
{
  SwapchainSupport s(device, surface);
  bool queueFamiliesSupported =
      std::all_of(queueFamilyIndices.begin(), queueFamilyIndices.end(),
                  [device, surface](auto index) {
                    return device.getSurfaceSupportKHR(index, surface);
                  });
  return !(s.formats.empty() || s.presentModes.empty()) && queueFamiliesSupported;
}

std::vector<vk::UniqueImage> getUniqueSwapchainImagesKHR(vk::Device& device,
                                                         vk::SwapchainKHR& swapchain)
{
  auto images = device.getSwapchainImagesKHR(swapchain);
  std::vector<vk::UniqueImage> result;
  result.reserve(images.size());
  for (auto& image : images) {
    result.emplace_back(vk::UniqueImage{image});
  }
  return std::move(result);
}

Holder::Holder(vk::PhysicalDevice& physicalDevice, vk::Device& device,
               vk::SurfaceKHR& surface)
    : swapchain(makeSwapchain(physicalDevice, device, surface)),
      format(SwapchainSupport(physicalDevice, surface).chooseFormat().format),
      extent(SwapchainSupport(physicalDevice, surface).chooseExtent()),
      physicalDevice(physicalDevice), device(device), surface(surface)
{
}

void Holder::recreateSwapchain(int width, int height)
{
  swapchain = makeSwapchain(physicalDevice, device, surface, swapchain.get());
  auto support = SwapchainSupport(physicalDevice, surface);
  format = support.chooseFormat().format;
  extent = support.chooseExtent(width, height);
}

} // namespace gh::detail::swapchain