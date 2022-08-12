/**
 * @file vulkan_helpers.h
 * @author ght365 (ght365@hotmail.com)
 * @brief @b generic AgosGE's header for @b Vulkan-oriented @c helper @c functions. You may also use it.
 * @version 0.1
 * @date 2021-08-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include "Agos/src/renderer/glfw/glfw_instance.h"

#include AG_VULKAN_INCLUDE
#include <memory>
#include <vector>

namespace Agos
{

namespace VulkanHandler
{

struct VulkanHelpers
{

    // * image helpers
    struct AgImage
    {
        VkImage         image;
        VkDeviceMemory  imageMemory;
        VkImageView     imageView;
    };

    // * framebuffers' attachment helpers
    using AgFramebufferAttachment = struct AgImage;

    // * image helpers
    static AgImage      create_image(
        const VkPhysicalDevice&         physical_device,
        const VkDevice&                 logical_device,
        const uint32_t&                 width,
        const uint32_t&                 height,
        const uint32_t&                 mip_levels,
        const VkSampleCountFlagBits&    num_samples,
        const VkFormat&                 format,
        const VkImageTiling&            tiling,
        const VkImageUsageFlags&        usage,
        const VkMemoryPropertyFlags&    properties,
        const VkAllocationCallbacks*    allocator);

    static VkImageView  create_image_view(
        const VkDevice&                 logical_device,
        const VkImage&                  image,
        const VkFormat&                 format,
        const VkImageAspectFlags&       aspect_mask,
        const uint32_t&                 mip_levels,
        const VkAllocationCallbacks*    allocator);

    static void         transition_image_layout(
        const VkDevice&                 logical_device,
        const VkQueue&                  graphics_queue,
        const VkCommandPool&            command_pool,
        const VkImage&                  image,
        const VkFormat&                 format,
        const VkImageLayout&            old_layout,
        const VkImageLayout&            new_layout,
        const uint32_t&                 mip_levels);

    static void         copy_buffer_to_image(
        const VkDevice&                 logical_device,
        const VkQueue&                  execution_queue,
        const VkCommandPool&            command_pool,
        const VkBuffer&                 buffer,
        const VkImage&                  image,
        const uint32_t&                 width,
        const uint32_t&                 height);

    static VkFormat     find_supported_format(
        const VkPhysicalDevice&         physical_device,
        const std::vector<VkFormat>&    candidates,
        VkImageTiling                   tiling,
        VkFormatFeatureFlags            features);

    static VkFormat     find_depth_format(
        const VkPhysicalDevice&         physical_device);

    static void         generate_mipmaps(
        const VkPhysicalDevice&         physical_device,
        const VkDevice&                 logical_device,
        const VkQueue&                  execution_queue,
        const VkCommandPool&            command_pool,
        const VkImage&                  image,
        const VkFormat&                 image_format,
        const int32_t&                  tex_width,
        const int32_t&                  tex_height,
        const uint32_t&                 mip_levels);
    // * image helpers

    // * buffer helpers
    struct AgBuffer
    {
        VkBuffer        buffer;
        VkDeviceMemory  bufferMemory;
    };

    static AgBuffer create_buffer(
        const VkPhysicalDevice&         physical_device,
        const VkDevice&                 logical_device,
        const VkDeviceSize&             size,
        const VkBufferUsageFlags&       usage,
        const VkMemoryPropertyFlags&    properties,
        const VkAllocationCallbacks*    allocator = nullptr);

    static void     copy_buffer(
        const VkDevice&                 logical_device,
        const VkQueue&                  execution_queue,
        const VkCommandPool&            command_pool,
        const VkBuffer&                 src_buffer,
        const VkBuffer&                 dst_buffer,
        const VkDeviceSize&             size,
        const VkAllocationCallbacks*    allocator = nullptr);

    // is just two lines of code... literally
    static void     destroy_buffer(
        const VkDevice&                 logical_device,
        const AgBuffer&                 buffer,
        const VkAllocationCallbacks*    allocator = nullptr);

    // one command buffer handlers
    static VkCommandBuffer                  begin_single_time_command(
                                                const VkDevice&         logical_device,
                                                const VkCommandPool&    command_pool);
    // end_single_time_command EXECUTES command buffer BEFORE DESTROYING
    static void                             end_single_time_command(
                                                const VkDevice&         logical_device,
                                                const VkCommandPool&    command_pool,
                                                const VkQueue&          execution_queue,
                                                const VkCommandBuffer&  command_buffer);
    // multiple command buffers handlers
    static std::vector<VkCommandBuffer>     begin_single_time_commands(
                                                const VkDevice&         logical_device,
                                                const VkCommandPool&    command_pool,
                                                const size_t&           command_buffers_count);
    // end_single_time_commands EXECUTES each command buffer BEFORE DESTROYING
    static void                             end_single_time_commands(
                                                const std::vector<VkCommandBuffer>&    command_buffers);
    // ** images, image views, and buffers helper functions

};  // * struct VulkanAppHelpers

}   // namespace VulkanHandler (within namespace Agos)

}   // namespace Agos
