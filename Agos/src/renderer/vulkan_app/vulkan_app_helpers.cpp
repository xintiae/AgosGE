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
    // vertex generic
    VkPipelineShaderStageCreateInfo genericVertShaderStageInfo{};
    vertShaderStageInfo.sType                       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module                      = vertShaderModule;
    vertShaderStageInfo.pName                       = "main";

    // vertex lighting
    VkPipelineShaderStageCreateInfo lightingVertShaderStageInfo{};

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType                       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage                       = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module                      = fragShaderModule;
    fragShaderStageInfo.pName                       = "main";

    VkPipelineShaderStageCreateInfo shaderStages[]  = {vertShaderStageInfo, fragShaderStageInfo};


    VkVertexInputBindingDescription bindingDescription                      = Agos::VulkanHandler::VulkanModeling::Vertex::get_binding_description();
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions  = Agos::VulkanHandler::VulkanModeling::Vertex::get_attribute_description();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType                             = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                          = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable            = VK_FALSE;


    VkViewport viewport{};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = static_cast<float>(m_SwapchainExtent.width);
    viewport.height     = static_cast<float>(m_SwapchainExtent.height);
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor{};
    scissor.offset      = {0, 0};
    scissor.extent      = m_SwapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount         = 1;
    viewportState.pViewports            = &viewport;
    viewportState.scissorCount          = 1;
    viewportState.pScissors             = &scissor;


    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable         = VK_FALSE;
    rasterizer.rasterizerDiscardEnable  = VK_FALSE;
    rasterizer.polygonMode              = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                = 1.0f;
    rasterizer.cullMode                 = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable          = VK_FALSE;


    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = m_MsaaSamples;


    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable        = VK_TRUE;
    depthStencil.depthWriteEnable       = VK_TRUE;
    depthStencil.depthCompareOp         = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable  = VK_FALSE;
    depthStencil.stencilTestEnable      = VK_FALSE;


    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable    = VK_FALSE;


    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType                 = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable         = VK_FALSE;
    colorBlending.logicOp               = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount       = 1;
    colorBlending.pAttachments          = &colorBlendAttachment;
    colorBlending.blendConstants[0]     = 0.0f;
    colorBlending.blendConstants[1]     = 0.0f;
    colorBlending.blendConstants[2]     = 0.0f;
    colorBlending.blendConstants[3]     = 0.0f;


    std::array<VkDescriptorSetLayout, 3> DSLs = { m_DSL_MVP_EL, m_DSL_Material, m_DSL_TexAmbDiffSpecNrmlMaps };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount   = DSLs.size();
    pipelineLayoutInfo.pSetLayouts      = DSLs.data();


    if (vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutInfo, m_Allocator, &m_GraphicsPipelineLayout) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerGraphicsPipelineManger - create_graphics_pipeline] Failed to create graphics pipeline layout!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE_LAYOUT;
    }


    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount             = 2;
    pipelineInfo.pStages                = shaderStages;
    pipelineInfo.pVertexInputState      = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState    = &inputAssembly;
    pipelineInfo.pViewportState         = &viewportState;
    pipelineInfo.pRasterizationState    = &rasterizer;
    pipelineInfo.pMultisampleState      = &multisampling;
    pipelineInfo.pDepthStencilState     = &depthStencil;
    pipelineInfo.pColorBlendState       = &colorBlending;
    pipelineInfo.layout                 = m_GraphicsPipelineLayout;
    pipelineInfo.renderPass             = m_RenderPass;
    pipelineInfo.subpass                = 0;
    pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE;


    if (vkCreateGraphicsPipelines(m_LogicalDevice, m_GraphicsPipelineCache, 1, &pipelineInfo, m_Allocator, &m_GraphicsPipeline) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/Agos::VulkanHandler::VulkanApp - create_graphics_pipeline] Failed to create graphics pipeline!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE;
    }
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
