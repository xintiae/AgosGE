#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"
#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_instance.h"
namespace Agos{
    class AgVulkanHandlerLogicalDevice;
}
#include "Agos/src/renderer/vulkan_logical_device.h"
namespace Agos{
    class AgVulkanHandlerSwapChain;
}
#include "Agos/src/renderer/vulkan_swapchain.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <optional>

namespace Agos
{
namespace VulkanPhysicalDevice
{
typedef struct AG_API AgQueueFamilyIndices
{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool isComplete()
    {
        return graphics_family.has_value() && present_family.has_value();
    }
} AgQueueFamilyIndices;

typedef struct AG_API AgSwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   present_modes;
} AgSwapChainSupportDetails;
}   // namespace VulkanPhysicalDevice (within namespace Agos)


typedef class AG_API AgVulkanHandlerPhysicalDevice
{
private:
    static const std::vector<const char*> m_DeviceExtensions;
    VkPhysicalDevice m_PhysicalDevice;
    VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;

public:
    AgVulkanHandlerPhysicalDevice();
    ~AgVulkanHandlerPhysicalDevice();

    AgVulkanHandlerPhysicalDevice(const AgVulkanHandlerPhysicalDevice& other)   = delete;
    AgVulkanHandlerPhysicalDevice(AgVulkanHandlerPhysicalDevice&& other)        = delete;

    AgResult pick_physical_device(
        const std::shared_ptr<AgVulkanHandlerInstance>& vulkan_instance,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);

    const std::vector<const char*>& get_device_extensions();
    VkPhysicalDevice& get_device();
    VkSampleCountFlagBits& get_msaa_samples();

    friend class Agos::AgVulkanHandlerLogicalDevice;
    friend class Agos::AgVulkanHandlerSwapChain;
private:
    bool is_device_suitable(
        const VkPhysicalDevice& physical_device,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);

    bool check_device_extensions_support(const VkPhysicalDevice& physical_device);

protected:
    static VulkanPhysicalDevice::AgQueueFamilyIndices find_queue_families(
        const VkPhysicalDevice& physical_device,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);

    VulkanPhysicalDevice::AgSwapChainSupportDetails query_swapchain_support(
        const VkPhysicalDevice& physical_device,
        const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance);
    
private:
    VkSampleCountFlagBits get_max_usable_sample_count();
};

}   // namespace Agos
