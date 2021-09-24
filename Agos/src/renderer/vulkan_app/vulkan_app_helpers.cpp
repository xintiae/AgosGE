#include "Agos/src/renderer/vulkan_app/vulkan_app_helpers.h"

#include "Agos/src/logger/logger.h"
#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"
#include AG_JSON_INCLUDE
#include <filesystem>
#include <fstream>


// ** = = = = = = = = = = = = = = = = = = = = graphics pipeline helper functions = = = = = = = = = = = = = = = = = = = =
// * Graphics PipelineS creates infos generation process
VkGraphicsPipelineCreateInfo Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::provide_generic_GP_create_info()
{

}

VkPipelineLayoutCreateInfo Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::populate_generic_GP_layout_create_info()
{

}

VkGraphicsPipelineCreateInfo Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::provide_lighting_GP_create_info()
{

}

VkPipelineLayoutCreateInfo Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::populate_lighting_GP_layout_create_info()
{

}
// * Graphics PipelineS creates infos generation process


std::array<VkShaderModule, 2> Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::create_shader_module(
    const VkDevice& logical_device,
    const std::string& shader_folder,
    const VkAllocationCallbacks* allocator)
{
    Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader shader = compile_shader(shader_folder);

    // generic then lighting shaders
    std::array<VkShaderModuleCreateInfo, 2> createsInfo{};
    createsInfo[0].sType        = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createsInfo[0].codeSize     = shader.generic_shader_contents.size();
    createsInfo[0].pCode        = reinterpret_cast<const uint32_t *>(shader.generic_shader_contents.data());

    createsInfo[1].sType        = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createsInfo[1].codeSize     = shader.generic_shader_contents.size();
    createsInfo[1].pCode        = reinterpret_cast<const uint32_t *>(shader.generic_shader_contents.data());

    // generic then lighting shaders
    std::array<VkShaderModule, 2> shaderModules;
    if (vkCreateShaderModule(logical_device, createsInfo.data(), allocator, shaderModules.data()) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: create_shader_module] Failed to create shader module!");
        return std::array<VkShaderModule, 2>({VK_NULL_HANDLE});
    }

    clear_compiled_shader(shader);
    return shaderModules;
}

/**
 * @param @std::string shader_folder_path
 * @b path to the @b dir containing a @b manifest.json (path without "/" at the end x] )
*/
Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader&& Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::compile_shader(const std::string& shader_folder_path)
{
    // read .json
    Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader shader_info = read_shader_manifold(shader_folder_path);

    // generic then lighting
    std::array<std::filesystem::path, 2> compiled_shaders_entry({shader_folder_path + "/" + shader_info.id_generic, shader_folder_path + "/" + shader_info.id_lighting});
    bool compiled_shaders_exist[2] = { false, false };

    // generic exists
    if ( std::filesystem::exists(compiled_shaders_entry[0]) )
    {
        AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Found possible already-compiled generic shader of type : " + type + " at location : "
            + shader_folder_path + "/" + shader_info.id_generic);
        compiled_shaders_exist[0] = true;
    }
    // lighting exists
    else if ( std::filesystem::exists(compiled_shaders_entry[1]) )
    {
        AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Found possible already-compiled lighting shader of type : " + type + " at location : "
            + shader_folder_path + "/" + shader_info.id_lighting);
        compiled_shaders_exist[1] = true;
    }
    else
    {
        AG_CORE_WARN("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] No pre-compiled shaders found at : " + shader_folder_path);
        // compiled_shaders_exist[0] = false;
        // compiled_shaders_exist[1] = false;
    }

    // compile
    // generic then lighting
    std::array<std::string, 2> compile_cmds = {
        "(cd " + shader_folder_path + " && " + shader_info.compile_generic + ")",
        "(cd " + shader_folder_path + " && " + shader_info.compile_lighting
    };

    if ( !( compiled_shaders_exist[0] || compiled_shaders_exist[1] || AG_SHADERS_SHALL_COMPILE)  )
    {
        // generic compilation
        AG_CORE_WARN("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Compiling generic shader : " + shader_folder_path + "/" + shader_info.id_generic + "...");
        int result = system(compile_cmds[0].c_str());
        if (result == 0)
            AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Done compiling.");
        else
            AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Compilation failed!");

        // lighting compilation
        AG_CORE_WARN("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Compiling lighting shader : " + shader_folder_path + "/" + shader_info.id_lighting + "...");
        int result = system(compile_cmds[1].c_str());
        if (result == 0)
            AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Done compiling.");
        else
            AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Compilation failed!");
    }

    // generic then lighting
    std::array<std::ifstream, 2> shaders_streams;
    shaders_streams[0] = std::move(std::ifstream(std::string(compiled_shaders_entry[0]), std::ios::ate | std::ios::binary));
    shaders_streams[1] = std::move(std::ifstream(std::string(compiled_shaders_entry[1]), std::ios::ate | std::ios::binary));

    if ( !shaders_streams[0].is_open() )
    {
        std::string err_msg = "[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Failed to open compiled generic shader file : " +
            shader_folder_path + "/" + shader_info.id_generic + "!";

        AG_CORE_CRITICAL(err_msg);
        throw std::runtime_error(err_msg);
    }
    else if ( !shaders_streams[1].is_open() )
    {
        std::string err_msg = "[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Failed to open compiled generic shader file : " +
            shader_folder_path + "/" + shader_info.id_lighting + "!";

        AG_CORE_CRITICAL(err_msg);
        throw std::runtime_error(err_msg);
    }

    size_t shaders_sizes[2] = { static_cast<size_t>(shaders_streams[0].tellg()), static_cast<size_t>(shaders_streams[1].tellg()) };
    std::vector<char> buffer;

    // generic
    buffer.resize(shaders_sizes[0]);
    shaders_streams[0].seekg(0);
    shaders_streams[0].read(buffer.data(), shaders_sizes[0]);
    shaders_streams[0].close();
    shader_info.generic_shader_contents = std::move(buffer);
    
    // lighting
    buffer.clear();
    buffer.resize(shaders_sizes[1]);
    shaders_streams[0].seekg(0);
    shaders_streams[0].read(buffer.data(), shaders_sizes[0]);
    shaders_streams[0].close();
    shader_info.lighting_shader_contents = std::move(buffer);

    return std::move(shader_info);
}

Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader&& Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::read_shader_manifold(const std::string& shader_folder_path)
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
    shader_info.folder_path             = shader_folder_path;
    shader_info.id_generic              = manifest["id_generic"];
    shader_info.id_lighting             = manifest["id_lighting"];
    shader_info.compile_generic         = manifest["compile_generic"];
    shader_info.compile_lighting        = manifest["compile_lighting"];

    establish_type(shader_info, manifest["type"]);

    manifest_stream.close();
    return std::move(shader_info);
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
        AG_CORE_ERROR("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: compile_shader] Shader type precised in manifest (which is type : \""
            + shader_type + "\") is unknown!");
    }
}

Agos::AgResult Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::clear_compiled_shader(const Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline::Shader& shader)
{
    // generic then lighting
    std::array<std::string, 2> delete_cmds ({
        std::string(AG_SHADER_DELETE_COMPILED_SHADER_COMMAND + std::string(" ") + shader.id_generic),
        std::string(AG_SHADER_DELETE_COMPILED_SHADER_COMMAND + std::string(" ") + shader.id_lighting)
    });

    // generics
    int result = system(delete_cmds[0].c_str());
    if (result != 0)
    {
        AG_CORE_ERROR("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: clear_compiled_shader] Failed to destroy compiled generic shader : " + shader.id_generic + "!");
        return AG_FAILED_TO_CLEAR_COMPILED_SHADER;
    }
    else
    {
        AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: clear_compiled_shader] Cleaned compiled generic shader : " + shader.id_generic);
    }

    // lighting
    int result = system(delete_cmds[1].c_str());
    if (result != 0)
    {
        AG_CORE_ERROR("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: clear_compiled_shader] Failed to destroy compiled lighting shader : " + shader.id_lighting + "!");
        return AG_FAILED_TO_CLEAR_COMPILED_SHADER;
    }
    else
    {
        AG_CORE_INFO("[Vulkan/Agos::VulkanHandler::VulkanAppHelpers::GraphicsPipeline:: clear_compiled_shader] Cleaned compiled lighting shader : " + shader.id_lighting);
    }
    
    return AG_SUCCESS;
}
// ** = = = = = = = = = = = = = = = = = = = = graphics pipeline helper functions = = = = = = = = = = = = = = = = = = = =
