/**
 * @file vulkan_app_helpers.h
 * @author ght365(ght365@hotmail.com)
 * @brief @b vulkan_app.h-specific Vulkan-based @c helper @c functions
 * @version 0.1
 * @date 2021-08-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

#include AG_VULKAN_INCLUDE
#include <string>
#include <vector>
#include <array>


namespace Agos
{

namespace VulkanHandler
{

namespace VulkanAppHelpers
{

namespace GraphicsPipeline
{
    // * Graphics Pipeline "Generation und Population" Stage
    static VkGraphicsPipelineCreateInfo     provide_generic_GP_create_info          (   const VkShaderModule& genVertShaderModule,
                                                                                        const VkShaderModule& genFragShaderModule,
                                                                                        const VkAllocationCallbacks* allocator);
    static VkPipelineLayoutCreateInfo       populate_generic_GP_layout_create_info  ();

    static VkGraphicsPipelineCreateInfo     provide_lighting_GP_create_info         (   const VkShaderModule& lhtVertShaderModule,
                                                                                        const VkShaderModule& lhtFragShaderModule,
                                                                                        const VkAllocationCallbacks* allocator);
    static VkPipelineLayoutCreateInfo       populate_lighting_GP_layout_create_info ();

    // * Shader helpers
    enum ShaderType : int8_t
    {
        __MIN_ENUM      = -128,

        __Error_type    = -1,
        __None          = 0,
        __Tessellation  = 1,
        __Geometry      = 2,
        __Vertex        = 3,
        __Fragment      = 4,
        __Compute       = 5,

        __MAX_ENUM      = 127
    };

    // relative to the manifest.json
    struct Shader
    {
        std::string         folder_path;
        std::vector<char>   generic_shader_contents;
        std::vector<char>   lighting_shader_contents;

        ShaderType          type;
        std::string         id_generic;
        std::string         id_lighting;
        std::string         compile_generic;
        std::string         compile_lighting;
    };

    // * Shader Modules
    static std::array<VkShaderModule, 2>    create_shader_module    (const VkDevice& logical_device, const std::string& shader_folder, const VkAllocationCallbacks* allocator);
    static Shader&&                         compile_shader          (const std::string& shader_folder_path);
    static Shader&&                         read_shader_manifold    (const std::string& shader_folder_path);
    static void                             establish_type          (Shader& shader_info, const std::string& shader_type);
    static AgResult                         clear_compiled_shader   (const Shader& shader);
};  // struct Graphics Pipeline


}   // namespace VulkanAppHelpers (within namespace Agos::VulkanHandler)

}   // namespace VulkanHander (within namespace Agos)

}   // namespace Agos
