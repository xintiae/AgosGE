#include "Agos/src/renderer/imgui/agos_imgui.h"

#include "Agos/src/renderer/vulkan_buffers.h"
#include "Agos/src/logger/logger.h"

extern VkDevice         AG_DEFAULT_LOGICAL_DEVICE_REFERENCE;

Agos::AgImGuiHandler::AgImGuiHandler(GLFWwindow*& application_window)
    :   m_LogicalDeviceReference(AG_DEFAULT_LOGICAL_DEVICE_REFERENCE),
        m_ApplicationWindowReference(application_window)
{
}

Agos::AgImGuiHandler::~AgImGuiHandler()
{
    terminate();
}

Agos::AgResult Agos::AgImGuiHandler::init(
    const std::shared_ptr<AgGLFWHandlerInstance>& glfw_instance,
    const std::shared_ptr<AgVulkanHandlerInstance>& vulkan_instance,
    const std::shared_ptr<AgVulkanHandlerPhysicalDevice>& physical_device,
    const std::shared_ptr<AgVulkanHandlerLogicalDevice>& logical_device,
    const std::shared_ptr<AgVulkanHandlerSwapChain>& swapchain,
    const std::shared_ptr<AgVulkanHandlerRenderPass>& renderpass,
    const std::shared_ptr<AgVulkanHandlerCommandPoolManager>& command_pool_manager
)
{
    m_LogicalDeviceReference        = logical_device->get_device();

    create_context();
    create_descriptor_pool(logical_device->get_device());

    ImGui_ImplGlfw_InitForVulkan(glfw_instance->get_window(), true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance           = vulkan_instance->get_instance();
    initInfo.PhysicalDevice     = physical_device->get_device();
    initInfo.Device             = logical_device->get_device();
    initInfo.QueueFamily        = physical_device->find_queue_families(physical_device->get_device(), glfw_instance).graphics_family.value();
    initInfo.Queue              = logical_device->get_graphics_queue();
    initInfo.MSAASamples        = physical_device->get_msaa_samples();
    initInfo.PipelineCache      = VK_NULL_HANDLE;
    initInfo.DescriptorPool     = m_ImguiDescriptorPool;
    initInfo.Allocator          = nullptr;
    initInfo.MinImageCount      = 2;
    initInfo.ImageCount         = swapchain->get_swapchain_images().size();
    initInfo.CheckVkResultFn    = Agos::ag_imgui_check_vk_result;
    ImGui_ImplVulkan_Init(&initInfo, renderpass->get_render_pass());

    upload_front_textures(
        logical_device->get_device(),
        logical_device->get_graphics_queue(),
        command_pool_manager->get_command_pool()
    );

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgImGuiHandler::update_ui()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();

    return AG_SUCCESS;
}

Agos::AgResult Agos::AgImGuiHandler::terminate()
{
    if (!m_ImguiTerminated)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        terminate_descriptor_pool();
        m_ImguiTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

Agos::AgResult Agos::AgImGuiHandler::create_descriptor_pool(
    const VkDevice& logical_device
)
{
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags            = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets          = 1000;
    createInfo.poolSizeCount    = std::size(poolSizes);
    createInfo.pPoolSizes       = poolSizes;

    if (vkCreateDescriptorPool(logical_device, &createInfo, nullptr, &m_ImguiDescriptorPool) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[ImGui - Vulkan/AgImGuiHandler - create_imgui_descriptor_pool] Failed to create ImGui's descriptor pool!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_POOL;
    }

    AG_CORE_INFO("[ImGui - Vulkan/AgImGuiHandler - create_imgui_descriptor_pool] Created ImGui's descriptor pool!");
    return AG_SUCCESS;
}

Agos::AgResult Agos::AgImGuiHandler::terminate_descriptor_pool()
{
    if (!m_ImguiDescriptorPoolTerminated)
    {
        vkDestroyDescriptorPool(m_LogicalDeviceReference, m_ImguiDescriptorPool, nullptr);
        m_ImguiDescriptorPoolTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}

void Agos::AgImGuiHandler::create_context()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();
}

void Agos::AgImGuiHandler::upload_front_textures(
    const VkDevice& logical_device,
    const VkQueue& graphics_queue,
    const VkCommandPool& command_pool
)
{
    VkCommandBuffer commandBuffer = Agos::AgVulkanHandlerCommandBufferManager::begin_single_time_command(
        logical_device,
        command_pool
    );
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    Agos::AgVulkanHandlerCommandBufferManager::end_single_time_command(
        logical_device,
        graphics_queue,
        command_pool,
        commandBuffer
    );
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Agos::ag_imgui_check_vk_result(VkResult err)
{
    if (err != VK_SUCCESS)
    {
        AG_CORE_ERROR("[ImGui/AgImGuiHandler - ag_imgui_check_vk_result] Error! VkResult : " + std::to_string(err));
    }
}

