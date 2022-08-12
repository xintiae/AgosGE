/**
 * @file vulkan_app.h
 * @author ght365 (ght365@hotmail.com)
 * @brief main AgosGE's Base Class Layer (BCL)
 * @version 0.1
 * @date 2021-08-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/renderer/debug_layers/vulkan_debug_layers.h"
#include "Agos/src/renderer/glfw/glfw_instance.h"
#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"

#include AG_GLFW_INCLUDE
#include AG_VULKAN_INCLUDE

#include <memory>
#include <optional>
#include <cstdint>


namespace Agos
{
namespace VulkanHandler
{

class AG_API VulkanBase
{
protected:
    std::shared_ptr<GLFWHandler::GLFWInstance>& m_GLFWInterfaceRef;

    VkInstance                  m_Instance; 
    // debugging tools
    VkDebugUtilsMessengerEXT    m_DebugMessenger;
    VkAllocationCallbacks*      m_Allocator;            // = NULL

    VkPhysicalDevice            m_PhysicalDevice;
    VkDevice                    m_LogicalDevice;

    VkSurfaceKHR                m_WindowSurface;

    // physical device's extensions & properties...
    static const std::vector<const char*> m_DeviceExtensions;
    VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;

    // logical device's different family queues...
    VkQueue                     m_GraphicsQueue;
    VkQueue                     m_PresentQueue;
    VkQueue                     m_TransferQueue;
    VkQueue                     m_ComputeQueue;

    bool                        m_FramebufferResizedFlag;   // = false

    // destructions tracking
    bool                        m_InstanceDestroyed;                // = false
    bool                        m_DebugMessengerTerminated;         // = false
    bool                        m_AllocatorTerminated;              // = false

    bool                        m_PhysicalDeviceDestroyed;          // = false
    bool                        m_LogicalDeviceDestroyed;           // = false
    bool                        m_WindowSurfaceDestroyed;           // = false

    // true when all of previously enumerated m_xxxDestroyed have turned true
    bool                        m_VulkanBaseTerminated;             // = false

public:
    explicit VulkanBase(std::shared_ptr<GLFWHandler::GLFWInstance>& glfw_instance);
    explicit VulkanBase(const VulkanBase& other)    = delete;
    explicit VulkanBase(VulkanBase&& other)         = delete;

    virtual ~VulkanBase();
    VulkanBase& operator=(const VulkanBase& other)  = delete;
    VulkanBase& operator=(VulkanBase&& other)       = delete;


    AgResult    setup_vulkan_base();
    AgResult    terminate_vulkan_base();

    friend class Agos::VulkanHandler::VulkanHelpers;
private:
    // setup_vulkan_base()
    // {
    AgResult    setup_allocation_callbacks();
    AgResult    create_instance();
    AgResult    setup_debug_messenger();
    AgResult    setup_window_surface();
    AgResult    pick_physical_device();
    AgResult    create_logical_device();
    // }

    // terminate_vulkan_base()
    // {
    AgResult    destroy_logical_device();
    AgResult    destroy_physical_device();
    AgResult    terimnate_window_surface();
    AgResult    terminate_debug_messenger();
    AgResult    destroy_instance();
    AgResult    terminate_allocation_callbacks();
    // }

protected:
    // physical device's extensions & properties (queue families, their indices, etc...)
    struct QueueFamiliesIndices
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
        std::optional<uint32_t> transfer_family;
        std::optional<uint32_t> compute_family;

        bool isComplete() const
        {
            return graphics_family.has_value() && present_family.has_value() && transfer_family.has_value() && compute_family.has_value();
        }
    };

    // different details about the swapchain and what it supports 
    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   present_modes;
    };

    // misc wrappers to help you out when querying infos about your GPU
    // suitable for graphics op (graphics and presentation queues, swapchain extension, MSAA)
    static bool                     is_device_suitable                  (const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface);
    static QueueFamiliesIndices     find_queue_families_indices         (const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface);
    static bool                     check_device_extensions_support     (const VkPhysicalDevice& physical_device);
    static SwapchainSupportDetails  query_swapchain_support             (const VkPhysicalDevice& physical_device, const VkSurfaceKHR& surface);
    static VkSampleCountFlagBits    get_max_usable_sample_count         (const VkPhysicalDevice& physical_device);
    static uint32_t                 find_memory_type_index              (const VkPhysicalDevice& physical_device, const uint32_t& type_filter, const VkMemoryPropertyFlags& properties);

};

}   // namespace VulkanHandler

}   // namespace Agos
