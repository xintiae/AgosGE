#include "Agos/src/renderer/vulkan_graphics_pipeline.h"

#include "Agos/src/logger/logger.h"
#include <fstream>
#include <filesystem>

Agos::AgVulkanHandlerGraphicsPipelineManager::AgVulkanHandlerGraphicsPipelineManager()
{
}

Agos::AgVulkanHandlerGraphicsPipelineManager::AgVulkanHandlerGraphicsPipelineManager(const VkDevice& logical_device)
{
    m_LogicalDeviceReference = logical_device;
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

    std::vector<char> vertShaderCode = read_file( shaders_path + "vert.spv" );
    std::vector<char> fragShaderCode = read_file( shaders_path + "frag.spv" );

    VkShaderModule vertShaderModule = create_shader_module(vertShaderCode, logical_device->get_device());
    VkShaderModule fragShaderModule = create_shader_module(fragShaderCode, logical_device->get_device());

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

    VkVertexInputBindingDescription bindingDescription = Agos::VulkanGraphicsPipeline::Vertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Agos::VulkanGraphicsPipeline::Vertex::getAttributeDescriptions();

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
    AG_CORE_INFO("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - create_graphics_pipeline] crated graphics pipeline!");
    return AG_SUCCESS;
}

// path to the file (e.g. /home/user1234/AgosGE/Agos/shaders/shader.vert)
Agos::AgResult Agos::AgVulkanHandlerGraphicsPipelineManager::compile_vertex_shader(const std::string& vert_shader_path)
{
    bool is_compiled = false;

    if (std::filesystem::exists(std::filesystem::path(vert_shader_path)))
    {
        AG_CORE_INFO("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_vertex_shader] Found possible vertex shader : " + vert_shader_path);
    }
    AG_CORE_WARN("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - compile_vertex_shader] Compiling file " + vert_shader_path + "...");
    std::string compile_cmd = std::string("./compile_vert_shader.") + std::string(AG_SHADER_SCRIPT_COMPILE_EXTENTION);
    system(compile_cmd.c_str());
    if (std::filesystem::exists(vert_shader_path));
}

Agos::AgResult Agos::AgVulkanHandlerGraphicsPipelineManager::compile_fragment_shader(const std::string& frag_shader_path)
{

}

std::vector<char> Agos::AgVulkanHandlerGraphicsPipelineManager::read_file(const std::string& file_path)
{
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        AG_CORE_ERROR("[Vulkan/AgVulkanHandlerGraphicsPipelineManager - read_file] Failed to open file! current path to file : " + std::string(file_path));
        return std::vector<char>();
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

Agos::AgResult Agos::AgVulkanHandlerGraphicsPipelineManager::terminate()
{
    if (!m_Terminated)
    {
        vkDestroyPipeline(m_LogicalDeviceReference, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_LogicalDeviceReference, m_GraphicsPipelineLayout, nullptr);
        m_Terminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

VkShaderModule Agos::AgVulkanHandlerGraphicsPipelineManager::create_shader_module(
    const std::vector<char>& shader_source,
    const VkDevice& logical_device)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader_source.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(shader_source.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logical_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}