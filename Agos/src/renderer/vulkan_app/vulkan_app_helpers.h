/**
 * @file vulkan_app_helpers.h
 * @author ght365(ght365@hotmail.com)
 * @brief @b vulkan_app.h-specific Vulkan-based @c helper @c functions. Relative to AgosGE, you may not want to use any of the bellow defined functions.
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

// ** namespace Graphics Pipeline (within namespace Agos::VulkanHandler::VulanAppHelpers) ========================================================
namespace GraphicsPipeline
{
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
        std::vector<char>   shader_contents;

        ShaderType          type;
        std::string         id;
        std::string         compile;
    };

    // * Shader Modules
    static VkShaderModule   create_shader_module               (const VkDevice&                logical_device,
                                                                const std::string&             shader_folder,
                                                                const VkAllocationCallbacks*   allocator            );
    static Shader           compile_shader_and_load_contents   (const std::string&             shader_folder_path   );
    static Shader           read_shader_manifold               (const std::string&             shader_folder_path   );
    static void             establish_type                     (Shader&                        shader_info,
                                                                const std::string&             shader_type          );
    static AgResult         clear_compiled_shader              (const Shader&                  shader               );
};
// ** namespace Graphics Pipeline (within namespace Agos::VulkanHandler::VulanAppHelpers) ========================================================


}   // namespace VulkanAppHelpers (within namespace Agos::VulkanHandler)

}   // namespace VulkanHander (within namespace Agos)

}   // namespace Agos


// ** @file Agos/src/renderer/vulkan_app/vulkan_app_helpers.cpp
#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"
#include "Agos/src/renderer/vulkan_app/vulkan_entity.h"
#include AG_JSON_INCLUDE
#include <filesystem>
#include <fstream>


// ** = = = = = = = = = = = = = = = = = = = = graphics pipeline helper functions = = = = = = = = = = = = = = = = = = = =
VkShaderModule Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::create_shader_module(
    const VkDevice& logical_device,
    const std::string& shader_folder,
    const VkAllocationCallbacks* allocator)
{
    Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader shader = compile_shader_and_load_contents(shader_folder);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType        = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize     = shader.shader_contents.size();
    createInfo.pCode        = reinterpret_cast<const uint32_t *>(shader.shader_contents.data());

    VkShaderModule shaderModule;

    if ( vkCreateShaderModule(logical_device, &createInfo, allocator, &shaderModule) != VK_SUCCESS )
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: create_shader_module] Failed to create shader module!");
        throw std::runtime_error("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: create_shader_module] Failed to create shader module!");
    }

    clear_compiled_shader(shader);
    return shaderModule;
}

/**
 * @param @std::string shader_folder_path
 * @b path to the @b dir containing a @b manifest.json (path without "/" at the end x] )
*/
Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::compile_shader_and_load_contents(const std::string& shader_folder_path)
{
    // ** read .json
    Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader shader_info = read_shader_manifold(shader_folder_path);

    std::filesystem::path compiled_shader_entry{shader_folder_path + "/" + shader_info.id};
    bool compiled_shader_exists = false;    

    if ( std::filesystem::exists(compiled_shader_entry) )
    {
        AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Found possible already-compiled generic shader of type : " + shader_info.type + " at location : "
            + shader_folder_path + "/" + shader_info.id);
        compiled_shader_exists = true;
    }
    else
    {
        AG_CORE_WARN("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] No pre-compiled shaders found at : " + shader_folder_path);
    }

    // ** compile
    std::string compile_cmd = { "(cd " + shader_folder_path + " && " + shader_info.compile + ")" };

    if ( (!compiled_shader_exists) || AG_SHADERS_SHALL_BE_COMPILED  )
    {
        AG_CORE_WARN("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Compiling shader : " + shader_folder_path + "/" + shader_info.id + "...");
        if (system(compile_cmd.c_str()) == 0)
            AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Done compiling.");
        else
            AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Compilation failed!");
    }

#ifdef WIN32 // On Windows, std::filesystem::path has to be converted differently
    std::ifstream shader_stream = std::move(std::ifstream(compiled_shader_entry.string(), std::ios::ate | std::ios::binary));
#else
    std::ifstream shader_stream = std::move(std::ifstream(std::string(compiled_shader_entry), std::ios::ate | std::ios::binary));
#endif

    if ( !shader_stream.is_open() )
    {
        std::string err_msg = "[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Failed to open compiled shader file : " +
            shader_folder_path + "/" + shader_info.id + "!";

        AG_CORE_CRITICAL(err_msg);
        throw std::runtime_error(err_msg);
    }

    size_t shader_size = static_cast<size_t>(shader_stream.tellg());

    std::vector<char> buffer(shader_size);
    shader_stream.seekg(0);
    shader_stream.read(buffer.data(), buffer.size());
    shader_stream.close();
    shader_info.shader_contents = std::move(buffer);
    
    return shader_info;
}

Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::read_shader_manifold(const std::string& shader_folder_path)
{
    // manifest path
    std::string     manifest_path   = shader_folder_path + "/manifest.json";
    // ifstream
    std::ifstream   manifest_stream (manifest_path);
    // contents
    std::string     manifest_content((std::istreambuf_iterator<char>(manifest_stream)), std::istreambuf_iterator<char>());

    // final parsing
    nlohmann::json  manifest = nlohmann::json::parse(manifest_content);

    // fill in shader's infos
    Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader shader_info;
    shader_info.folder_path     = shader_folder_path;
    shader_info.id              = manifest["id"];
    shader_info.compile         = manifest["compile"];
    establish_type(shader_info, manifest["type"]);

    manifest_stream.close();
    return shader_info;
}

void Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::establish_type(Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader& shader_info, const std::string& shader_type)
{
    if          (shader_type == "none")
    {
        shader_info.type = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::ShaderType::__None;
    }
    else if     (shader_type == "tessellation")
    {
        shader_info.type = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::ShaderType::__Tessellation;
    }
    else if     (shader_type == "geometry")
    {

        shader_info.type = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::ShaderType::__Geometry;
    }
    else if     (shader_type == "vertex")
    {
        shader_info.type = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::ShaderType::__Vertex;
    }
    else if     (shader_type == "fragment")
    {
        shader_info.type = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::ShaderType::__Fragment;
    }
    else if     (shader_type == "compute")
    {
        shader_info.type = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::ShaderType::__Compute;
    }
    else
    {
        shader_info.type = Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::ShaderType::__Error_type;
        AG_CORE_ERROR("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Shader type precised in manifest (which is : \""
            + shader_type + "\") is unknown!");
    }
}

Agos::AgResult Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::clear_compiled_shader(const Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader& shader)
{
    std::string delete_cmd { std::string(AG_SHADER_DELETE_COMPILED_SHADER_COMMAND + std::string(" ") + shader.folder_path + std::string("/") + shader.id) };

    if (system(delete_cmd.c_str()) != 0)
    {
        AG_CORE_ERROR("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: clear_compiled_shader] Failed to destroy compiled shader : " + shader.id + "!");
        return AG_FAILED_TO_CLEAR_COMPILED_SHADER;
    }
    AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: clear_compiled_shader] Cleaned compiled shader : " + shader.id);
    
    return AG_SUCCESS;
}
// ** = = = = = = = = = = = = = = = = = = = = graphics pipeline helper functions = = = = = = = = = = = = = = = = = = = =
