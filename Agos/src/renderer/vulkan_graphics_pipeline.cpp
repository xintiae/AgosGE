#include "Agos/src/renderer/vulkan_graphics_pipeline.h"

#include AG_JSON_INCLUDE
#include "Agos/src/logger/logger.h"
#include <fstream>
#include <filesystem>

extern VkDevice        AG_DEFAULT_LOGICAL_DEVICE_REFERENCE;

Agos::AgVulkanHandlerGraphicsPipelineManager::AgVulkanHandlerGraphicsPipelineManager()
    : m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE)
{
}

Agos::AgVulkanHandlerGraphicsPipelineManager::AgVulkanHandlerGraphicsPipelineManager(VkDevice& logical_device)
    : m_LogicalDeviceReference(logical_device)
{
}

Agos::AgVulkanHandlerGraphicsPipelineManager::~AgVulkanHandlerGraphicsPipelineManager()
{
    terminate();
}

Agos::AgResult Agos::AgVulkanHandlerGraphicsPipelineManager::create_graphics_pipeline(
    const std::string& shaders_path,
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::shared_ptr<AgVulkanHandlerRenderPass>& render_pass,
    const std::shared_ptr<AgVulkanHandlerDescriptorManager>& descriptor)
{
    m_LogicalDeviceReference = logical_device->get_device();

    VkShaderModule vertShaderModule = create_shader_module(shaders_path + AG_DEFAULT_VERTEX_SHADER_FOLDER, logical_device->get_device());
    VkShaderModule fragShaderModule = create_shader_module(shaders_path + AG_DEFAULT_FRAGMENT_SHADER_FOLDER, logical_device->get_device());

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkVertexInputBindingDescription bindingDescription = Agos::VulkanGraphicsPipeline::Vertex::get_binding_description();
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Agos::VulkanGraphicsPipeline::Vertex::get_attribute_description();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain->get_swapchain_extent().width);
    viewport.height = static_cast<float>(swapchain->get_swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->get_swapchain_extent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = physical_device->get_msaa_samples();

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptor->get_descriptor_set_layout();

    if (vkCreatePipelineLayout(logical_device->get_device(), &pipelineLayoutInfo, nullptr, &m_GraphicsPipelineLayout) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerGraphicsPipelineManger - create_graphics_pipeline] Failed to create graphics pipeline layout!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE_LAYOUT;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_GraphicsPipelineLayout;
    pipelineInfo.renderPass = render_pass->get_render_pass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(logical_device->get_device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - create_graphics_pipeline] Failed to create graphics pipeline!");
        return AG_FAILED_TO_CREATE_GRAPHICS_PIPELINE;
    }

    vkDestroyShaderModule(logical_device->get_device(), fragShaderModule, nullptr);
    vkDestroyShaderModule(logical_device->get_device(), vertShaderModule, nullptr);
    AG_CORE_INFO("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - create_graphics_pipeline] created graphics pipeline!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgVulkanHandlerGraphicsPipelineManager::terminate(const bool& mark_as_terminated)
{
    if (!m_Terminated)
    {
        vkDestroyPipeline(m_LogicalDeviceReference, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_LogicalDeviceReference, m_GraphicsPipelineLayout, nullptr);
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - terminate] Destroyed graphics pipeline and graphics pipeline layout!");
        m_Terminated = mark_as_terminated;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkPipeline& Agos::AgVulkanHandlerGraphicsPipelineManager::get_graphics_pipeline()
{
    return m_GraphicsPipeline;
}

VkPipelineLayout& Agos::AgVulkanHandlerGraphicsPipelineManager::get_graphics_pipeline_layout()
{
    return m_GraphicsPipelineLayout;
}

VkShaderModule Agos::AgVulkanHandlerGraphicsPipelineManager::create_shader_module(
    const std::string& shader_folder,
    const VkDevice& logical_device)
{
    Agos::VulkanGraphicsPipeline::Shader shader = compile_shader(shader_folder);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader.shader_contents.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(shader.shader_contents.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logical_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - create_shader_module] Failed to create shader module!");
        return VK_NULL_HANDLE;
    }

    clear_compiled_shader(shader);
    return shaderModule;
}

/**
 * @param @std::string shader_folder_path
 * @b path to the @b dir containing both @b manifest.json @c and @b <your_vertex_shader_name> (path without "/" at the end x] )
*/
Agos::VulkanGraphicsPipeline::Shader Agos::AgVulkanHandlerGraphicsPipelineManager::compile_shader(const std::string& shader_folder_path)
{
    // read .json
    std::string manifest_path = shader_folder_path + "/manifest.json";
    std::ifstream manifest_stream(manifest_path);
    std::string manifest_content((std::istreambuf_iterator<char>(manifest_stream)), std::istreambuf_iterator<char>());
    nlohmann::json manifest = nlohmann::json::parse(manifest_content);

    Agos::VulkanGraphicsPipeline::Shader shader_info;
    std::string type = manifest["type"];
    shader_info.folder_path = shader_folder_path;
    shader_info.id          = manifest["id"];
    shader_info.id_compiled = manifest["id_compiled"];
    shader_info.compile     = manifest["compile"];
    std::string compiled_shader_path = shader_folder_path + "/" + shader_info.id_compiled;

    if (type == "none")
        shader_info.type = Agos::VulkanGraphicsPipeline::ShaderTypes::__None;
    else if (type == "vertex")
        shader_info.type = Agos::VulkanGraphicsPipeline::ShaderTypes::__Vertex;
    else if (type == "fragment")
        shader_info.type = Agos::VulkanGraphicsPipeline::ShaderTypes::__Fragment;
    else
    {
        shader_info.type = Agos::VulkanGraphicsPipeline::ShaderTypes::__Error_type;
        AG_CORE_ERROR("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_shader] Shader type precised in manifest : "
            + manifest_path + " is unknown!");
    }

    bool compiled_shader_exists = false;
    std::filesystem::path compiled_shader_entry(compiled_shader_path);

    if (std::filesystem::exists(compiled_shader_entry) )
    {
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_shader] Found possible compiled shader of type : " + type + " : "
            + shader_folder_path + "/" + shader_info.id_compiled);
        compiled_shader_exists = true;
    }
    else
    {
        AG_CORE_WARN("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_shader] No compiled shader found at : " + shader_folder_path);
        // compiled_shader_exists = false;
    }

    // compile
    std::string compile_cmd = "(cd " + shader_folder_path + " && " + shader_info.compile + ")";
    if (!compiled_shader_exists || AG_SHADERS_COMPILE_ANYWAY)
    {
        AG_CORE_WARN("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_shader] Compiling shader : " + shader_folder_path + "/" + shader_info.id + "...");

        int result = system(compile_cmd.c_str());
        if (result == 0)
            AG_CORE_INFO("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_shader] Done compiling.");
        else
            AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_shader] Compilation failed!");
    }

    std::ifstream shader_stream(shader_folder_path + "/" + shader_info.id_compiled, std::ios::ate | std::ios::binary);
    if (!shader_stream.is_open())
    {
        AG_CORE_CRITICAL("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_shader] Failed to open compiled shader file : " +
            shader_folder_path + "/" + shader_info.id + "!");
    }

    size_t shader_size = static_cast<size_t>(shader_stream.tellg());
    std::vector<char> buffer(shader_size);

    shader_stream.seekg(0);
    shader_stream.read(buffer.data(), shader_size);

    shader_stream.close();
    manifest_stream.close();

    shader_info.shader_contents = std::move(buffer);
    return shader_info;
}

Agos::AgResult Agos::AgVulkanHandlerGraphicsPipelineManager::clear_compiled_shader(const Agos::VulkanGraphicsPipeline::Shader& shader)
{
    std::string compiled_shader_path = shader.folder_path + "/" + shader.id_compiled;
    std::string delete_cmd = AG_SHADER_DELETE_COMPILED_SHADER_COMMAND + std::string(" ") + compiled_shader_path;
    int result = system(delete_cmd.c_str());
    
    if (result != 0)
    {
        AG_CORE_ERROR("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - clear_compiled_shader] Failed to destroy compiled shader : " + compiled_shader_path + "!");
        return AG_FAILED_TO_CLEAR_COMPILED_SHADER;
    }
    else
    {
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - clear_compiled_shader] Cleaned compiled shader : " + compiled_shader_path);
    }

    return AG_SUCCESS;
}
