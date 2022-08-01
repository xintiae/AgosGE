#include "Agos/src/renderer/imgui/vulkan/vulkan_imgui.h"

#include "Agos/src/renderer/vulkan_app/vulkan_helpers.h"

// ** Vulkan-based Renderer ===============================================================================================

// ** Vulkan-based ImGui ==================================================================================================

// * = = = = = = = = = = = = = = = = = = = = constructors and destructors = = = = = = = = = = = = = = = = = = = =
Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::ImGuiInstance()
    : m_IniPath(AG_SOURCE_PATH + std::string(AG_IMGUI_INI_FILE_PATH)),
    m_ImGuiTerminated(false)
{
}

Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::~ImGuiInstance()
{
    terminate();
}
// * = = = = = = = = = = = = = = = = = = = = constructors and destructors = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = init, new_frame, draw_frame and terminate = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::init(
    Agos::ImGuiHandler::ImGuiVulkan::ImGui_initInfo* init_info
)
{
    m_LogicalDeviceRef  = init_info->Device;
    m_AllocatorRef      = init_info->Allocator;
    this->create_descriptor_pool();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = m_IniPath.c_str();
    ImGui::StyleColorsDark();


    ImGui_ImplGlfw_InitForVulkan(init_info->Window, true);
    ImGui_ImplVulkan_InitInfo imgui_init_info{};
    imgui_init_info.Instance            = init_info->Instance;
    imgui_init_info.PhysicalDevice      = init_info->PhysicalDevice;
    imgui_init_info.Device              = *init_info->Device;
    imgui_init_info.QueueFamily         = init_info->QueueFamily;
    imgui_init_info.Queue               = init_info->Queue;
    imgui_init_info.PipelineCache       = init_info->PipelineCache;
    imgui_init_info.DescriptorPool      = m_DescriptorPool;
    imgui_init_info.Subpass             = init_info->Subpass;
    imgui_init_info.MinImageCount       = init_info->MinImageCount;
    imgui_init_info.ImageCount          = init_info->ImageCount;
    imgui_init_info.MSAASamples         = init_info->MSAASamples;
    imgui_init_info.Allocator           = init_info->Allocator;
    imgui_init_info.CheckVkResultFn     = init_info->CheckVkResultFn;
    ImGui_ImplVulkan_Init(&imgui_init_info, init_info->RenderPass);


    VkCommandBuffer fonts_cmd_buffers = Agos::VulkanHandler::VulkanHelpers::begin_single_time_command(
        *init_info->Device,
        init_info->CommandPool
    );
    ImGui_ImplVulkan_CreateFontsTexture(fonts_cmd_buffers);
    Agos::VulkanHandler::VulkanHelpers::end_single_time_command(
        *init_info->Device,
        init_info->CommandPool,
        init_info->Queue,
        fonts_cmd_buffers
    );
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    return AG_SUCCESS;
}

Agos::AgResult Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::new_frame()
{
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    return AG_SUCCESS;
}

Agos::AgResult Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::end_frame()
{
    ImGui::Render();
    ImGui::UpdatePlatformWindows();
    return AG_SUCCESS;
}

Agos::AgResult Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::draw_frame(
    const VkCommandBuffer& command_buffer
)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    return AG_SUCCESS;
}

Agos::AgResult Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::terminate()
{
    if (!m_ImGuiTerminated)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        destroy_descriptor_pool();
        m_LogicalDeviceRef  = NULL;
        m_AllocatorRef      = NULL;
        m_ImGuiTerminated = true;
        return AG_SUCCESS;
    }
    return AG_INSTANCE_ALREADY_TERMINATED;
}
// * = = = = = = = = = = = = = = = = = = = = init, new_frame, draw_frame and terminate = = = = = = = = = = = = = = = = = = = =

// * = = = = = = = = = = = = = = = = = = = = descriptor pool = = = = = = = = = = = = = = = = = = = =
Agos::AgResult Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::create_descriptor_pool()
{
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER                    , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER     , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE              , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE              , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER       , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER       , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER             , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER             , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC     , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC     , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES},
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT           , AG_VULKAN_IMGUI_DESCRIPTOR_POOL_SIZES}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags          = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets        = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount  = (uint32_t)IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes     = poolSizes;

    if (vkCreateDescriptorPool(*m_LogicalDeviceRef, &poolInfo, m_AllocatorRef, &m_DescriptorPool) != VK_SUCCESS)
    {
        AG_CORE_CRITICAL("[ImGuiVulkan/ImGuiHandler::ImGuiVulkan::ImGuiInstance - create_descriptor_pool] Failed to create ImGui descriptor pool!");
        return AG_FAILED_TO_CREATE_DESCRIPTOR_POOL;
    }
    return AG_SUCCESS;
}

Agos::AgResult Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance::destroy_descriptor_pool()
{
    vkDestroyDescriptorPool(*m_LogicalDeviceRef, m_DescriptorPool, m_AllocatorRef);
    return AG_SUCCESS;
}
// * = = = = = = = = = = = = = = = = = = = = descriptor pool = = = = = = = = = = = = = = = = = = = =
