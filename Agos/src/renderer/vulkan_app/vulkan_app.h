/**
 * @file vulkan_app.h
 * @author ght365 (ght365@hotmail.com)
 * @brief main AgosGE's Application Class Layer (ACL)
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
#include "Agos/src/renderer/vulkan_base/vulkan_base.h"
#include "Agos/src/renderer/vulkan_app/vulkan_entity.h"
#include "Agos/src/renderer/camera/camera.h"
#include "Agos/src/renderer/imgui/agos_imgui.h"
#include "Agos/src/entities/entities.h"
#include "Agos/src/scene_manager/scene_manager.h"

#include AG_VULKAN_INCLUDE
#include AG_GLM_INCLUDE
#include <memory>
#include <vector>
#include <string>

namespace Agos
{

namespace VulkanHandler
{
class AG_API VulkanApp : VulkanBase
{
private:
    VkSwapchainKHR                              m_Swapchain;
    VkFormat                                    m_SwapchainFormat;
    VkExtent2D                                  m_SwapchainExtent;
    std::vector<VkImage>                        m_SwapchainImages;
    std::vector<VkImageView>                    m_SwapchainImageViews;
    std::vector<VkFramebuffer>                  m_SwapchainFramebuffers;

    // * I'll write up an architecture explaining readme, but for now, consider this
    // * AgosGE's scene is rendered to an offscreen target, which target is then used by ImGui to draw AgosGE's scene to a viewport
    // following render pass is ImGui dedicated; it'll be attached to m_SwapchainFramebuffers
    VkRenderPass                                m_RenderPass;

    // ** AgosGE's offscreen render target
    // following render pass is AgosGE's scene dedicated; it'll be attached to m_OffscreenFramebuffer
    VkRenderPass                                m_OffscreenRenderPass;
    // offscreen image res
    VkImage                                     m_OffscreenImage;
    VkDeviceMemory                              m_OffscreenImageMemory;
    VkImageView                                 m_OffscreenImageView;
    VkFramebuffer                               m_OffscreenFramebuffer;
    VkSampler                                   m_OffscreenSampler;
    VkExtent2D                                  m_OffscreenExtent;      // related to AG_DEFAUT_VIEWPORT_WIDTH and AG_DEFAUT_VIEWPORT_HEIGHT
    VkFormat                                    m_OffscreenFormat;      // = VK_FORMAT_R8G8B8A8_SRGB

    ImTextureID                                 m_OffscreenImGuiID;
    ImVec2                                      m_OffscreenViewportSize;
    // depth res
    VkImage                                     m_DepthImage;
    VkDeviceMemory                              m_DepthImageMemory;
    VkImageView                                 m_DepthImageView;
    // color res for MSAA
    VkImage                                     m_ColorImage;
    VkDeviceMemory                              m_ColorImageMemory;
    VkImageView                                 m_ColorImageView;


    // m_GraphicsPipelineCache                  m_GPC
    // m_GraphicsPipeline                       m_GP
    // ? later on : m_ComputePipeline           m_CP
    VkPipelineLayout                            m_GraphicsPipelineLayout;
    VkPipelineCache                             m_GraphicsPipelineCache;
    // m_GraphicsPipeline IS TO BE CONSIDERED AS AN OFFSCREEN GRAPHICS PIPELINE
    VkPipeline                                  m_GraphicsPipeline;

    // * entity rendering
    std::vector<std::shared_ptr<Agos::Entities::Entity>>    m_ToRenderEntities;
    std::vector<VulkanHelpers::AgBuffer>                    m_EntitiesVertexBuffers;
    std::vector<VulkanHelpers::AgBuffer>                    m_EntitiesIndexBuffer;

    // DescriptorSetLayout  DSL
    // DescriptorSet(s)     DS(s)
    VkDescriptorSetLayout                       m_DSL_MVP_EL;
    VkDescriptorSetLayout                       m_DSL_Material;
    VkDescriptorSetLayout                       m_DSL_TexAmbDiffSpecNrmlMaps;

    VkDescriptorPool                            m_DescriptorPool;
    // // per entity per swap chain images descriptor sets

    // per entity descriptor sets
    std::vector<VkDescriptorSet>                m_DS_MVP_EL;
    std::vector<VkDescriptorSet>                m_DS_Material;
    std::vector<VkDescriptorSet>                m_DS_TextAmbtDiffSpecNrmlMaps;

    // m_UniformBufferObjects                   m_UBO
    // // per swap chain images uniform buffers

    VulkanHelpers::AgBuffer                     m_UBO_MVP_EL;
    VulkanHelpers::AgBuffer                     m_UBO_Material;

    // per swapchain framebuffers command buffers
    std::vector<VkCommandBuffer>                m_CommandBuffers;
    VkCommandPool                               m_CommandBuffersPool;

    // sync objects - per swap chain images semaphores and fences
    std::vector<VkSemaphore>                    m_ImageAvailableSemaphores;
    std::vector<VkSemaphore>                    m_RenderFinishedSemaphores;
    std::vector<VkFence>                        m_InFlightFences;
    std::vector<VkFence>                        m_ImagesInFlight;

    size_t                                      m_CurrentFrame;

    // ** ImGui render target
    std::unique_ptr<Agos::ImGuiHandler::ImGuiVulkan::ImGuiInstance> m_ImGuiInterface;

    // current scene's status is important since it tells us how to build our command buffers / manage our GUI
    std::shared_ptr<Agos::SceneManager::SceneStatus>    m_SceneState;
    // !*!*! cf agos.h
    std::shared_ptr<Agos::Clipping::CameraObject>       m_ViewportCam;
    bool                                                m_ViewportCamShallMove = false;  // = false

    // destructions tracking
    bool                        m_SwapchainDestroyed;               // = false
    bool                        m_SwapchainImagesDestroyed;         // = false
    bool                        m_SwapchainImageViewsDestroyed;     // = false
    bool                        m_SwapchainFramebuffersDestroyed;   // = false

    bool                        m_RenderPassDestroyed;              // = false

    bool                        m_OffscreenRenderPassDestroyed;     // = false
    bool                        m_OffscreenImageResDestroyed;       // = false
    bool                        m_OffscreenFramebufferDestoyed;     // = false
    bool                        m_OffscreenSamplerDestroyed;        // = false
    bool                        m_ColorResDestroyed;                // = false
    bool                        m_DepthResDestroyed;                // = false

    bool                        m_GraphicsPipelineLayoutDestroyed;  // = false
    bool                        m_GraphicsPipelineCacheDestroyed;   // = false
    bool                        m_GraphicsPipelineDestroyed;        // = false

    bool                        m_DescriptorSetLayoutDestroyed;     // = false
    bool                        m_DescriptorPoolDestroyed;          // = false
    bool                        m_UniformBuffersDestroyed;          // = false
    bool                        m_EntitiesVertexBuffersDestroyed;   // = false
    bool                        m_EntitiesIndexBufferDestroyed;     // = false
    bool                        m_EntitiesTexturesUnloaded;         // = false

    bool                        m_CommandBuffersDestroyed;          // = false
    bool                        m_CommandBuffersPoolDestroyed;      // = false
    bool                        m_SyncObjectsDestroyed;             // = false

    bool                        m_OffscreenRenderTargetDestroyed;   // = false
    bool                        m_SwapchainAlreadyTerminated;       // = false
    // true when all of previously enumerated m_xxxDestroyed have turned true
    bool                        m_VulkanAppTerminated;              // = false

public:
    explicit VulkanApp(std::shared_ptr<GLFWHandler::GLFWInstance>& glfw_instance);
    explicit VulkanApp(const VulkanApp& other)      = delete;
    explicit VulkanApp(VulkanApp&& other)           = delete;

    ~VulkanApp();
    VulkanApp& operator=(const VulkanApp& other)    = delete;
    VulkanApp& operator=(VulkanApp&& other)         = delete;

    // ================================
    AgResult    init_vulkan_app     ();
    // ================================
    // loads the specified entities onto the GPU, and saves them for later draw calls - meaning they'll be drawn
    AgResult    load_entities       (const std::vector<std::shared_ptr<Agos::Entities::Entity>>& entities_to_render);
    AgResult    query_scene_state   (const std::shared_ptr<Agos::SceneManager::SceneStatus>& scene_status);
    AgResult    set_viewport        (const std::shared_ptr<Agos::Clipping::CameraObject>& viewport_cam);
    // drawing a frame consists of drawning the gui and the viewport
    // ================================
    AgResult    draw_frame          ();
    // ================================
    // clears m_ToRenderEntities
    AgResult    unload_entities     ();
    AgResult    terminate_vulkan_app();
    // ================================

protected:
    AgResult    recreate_swapchain  ();
    AgResult    process_events      (const Agos::GLFWHandler::GLFWEvent::Event& event);

private:
    AgResult    terminate_swapchain ();

    // ** =============================================
    // **   AgResult     init_vulkan_app()
    //      {
    //          setup_vulkan_base                   ();
    //          ---------------------------------------
    AgResult    create_swapchain                    ();
    AgResult    create_swapchain_image_views        ();
    AgResult    create_command_buffers_pool         ();
    AgResult    create_descriptor_pool              ();
    AgResult    create_uniform_buffers              ();
    AgResult    create_descriptor_sets_layout       ();
    AgResult    create_sync_objects                 ();
    //          ---------------------------------------
    AgResult    create_offscreen_render_target      ();
    //          {
    AgResult        create_offscreen_image_res      ();
    AgResult        create_depth_res                ();
    AgResult        create_color_res                ();
    AgResult        create_offscreen_render_pass    ();
    AgResult        create_offscreen_framebuffer    ();
    AgResult        create_offscreen_sampler        ();
    AgResult        create_graphics_pipeline        ();
    //          }
    //          ---------------------------------------
    AgResult    create_imgui_render_target          ();
    //          {
    AgResult        create_render_pass              ();
    AgResult        create_swapchain_framebuffers   ();
    //          }
    AgResult    init_vulkan_imgui                   ();
    //          {
    AgResult        init_imgui_interface            ();
    AgResult        add_imgui_textures              ();
    //          }
    //          ---------------------------------------
    //      }
    // **   AgResult    init_vulkan_imgui();
    // ** =============================================

    // ** ==========================================================================================
    // **   AgResult    load_entities(const std::vector<Agos::Entities::Entity>& entities_to_render)
    //      {
    AgResult    load_entities_textures              ();
    AgResult    create_vertex_buffers               ();
    AgResult    create_index_buffers                ();
    AgResult    create_descriptor_sets              ();
    //      }

    // NOTE ! draw_imgui: if scene_status.shall_draw_viewport,
    // NOTE ! proceed to building viewport ImGui-side and proceed to offscreen command buffers recording
    // **   AgResult    draw_frame()
    //      {
    // checks if the load_entities specified entities are still valid for the GPU to draw them (i.e. if an entity's signatured as destroyed, unload it from gpu and proceed to drawning "without it")
    AgResult    update_entities                     ();
    uint32_t    acquire_next_swapchain_image        ();
    AgResult    draw_imgui_objects                  (); // see at the end of class VulkanApp
    AgResult    update_vertex_buffers               ();
    AgResult    update_index_buffers                ();
    AgResult    update_uniform_buffers              ();
    AgResult    record_command_buffers              (const uint32_t& image_index);
    AgResult    submit_image_for_presentation       (const uint32_t& image_index);
    //      }
    // ** ==========================================================================================

    // ** ==========================================================================================
    // **   AgResult     terminate_vulkan_app()
    //      {
    //          -------------------------------------------textureWindowSize-----------------------------------------
    // *        AgResult     terminate_swapchain    ()
    //          {
    AgResult        destroy_swapchain_framebuffers  ();
    AgResult        destroy_command_buffers         ();
    AgResult        destroy_render_pass             ();
    AgResult        destroy_swapchain_image_views   ();
    AgResult        destroy_swapchain               ();
    //          }
    //          ------------------------------------------------------------------------------------
    //          m_ImGuiInterface->terminate         ();
    //          ------------------------------------------------------------------------------------
    AgResult    destroy_offscreen_render_target     ();
    //          {
    AgResult        unload_entities_textures        ();
    AgResult        destroy_offscreen_framebuffer   ();
    AgResult        destroy_graphics_pipeline       ();
    AgResult        destroy_offscreen_render_pass   ();
    AgResult        destroy_offscreen_sampler       ();
    AgResult        destroy_offscreen_image_res     ();
    AgResult        destroy_depth_res               ();
    AgResult        destroy_color_res               ();
    //          }
    //          ------------------------------------------------------------------------------------

    //          ------------------------------------------------------------------------------------
    AgResult    destroy_sync_objects                ();

    AgResult    destroy_vertex_buffers              ();
    AgResult    destroy_index_buffers               ();
    AgResult    destroy_uniform_buffers             ();

    AgResult    destroy_descriptor_pool             ();
    AgResult    destroy_command_buffers_pool        ();
    AgResult    destroy_decriptor_sets_layout       ();
    //          ------------------------------------------------------------------------------------
    //          terminate_vulkan_base               ();
    //      }
    // ** ==========================================================================================


    // ** = = = = = VulkanApp Helpers = = = = =
    // ** swapchain helpers
    static VkSurfaceFormatKHR   choose_swap_surface_format  (const std::vector<VkSurfaceFormatKHR>&     available_formats       );
    static VkPresentModeKHR     choose_swap_present_mode    (const std::vector<VkPresentModeKHR>&       available_present_modes );
    static VkExtent2D           choose_swap_extent          (const VkSurfaceCapabilitiesKHR&            capabilities,
                                                             GLFWwindow*                                glfw_instance           );

    // ** depth res helpers
    static VkFormat             find_depth_format           (const VkPhysicalDevice&                    physical_device         );
    static VkFormat             find_supported_format       (const std::vector<VkFormat>&               candidates,
                                                             const VkImageTiling&                       tiling,
                                                             const VkFormatFeatureFlags&                features,
                                                             const VkPhysicalDevice&                    physical_device         );

    // ** graphics pipeline heplers
    AgResult                    create_graphics_pipeline_layout     ();
    AgResult                    create_graphics_pipeline             (const VkShaderModule& vertex_shader_module,
                                                                     const VkShaderModule& fragment_shader_module);
    // see @file Agos/src/renderer/vulkan_app/vulkan_app_helpers.h


    // ** UBOs helpers
    // minUniformBufferOffsetAlighment limit constraints must be met
    VkDeviceSize                calculate_ubo_mvp_el_buffer_size();

    // ** descriptors heplers - uniform buffer objects for MVP matrices, lighting, materials, combined image samplers, etc...
    // allocate each swap chain images descriptor sets for each entity
    AgResult                    allocate_descriptor_sets_for_entity (const size_t& entity_index);
    // update descriptor sets of each swap chain images for each entity
    AgResult                    update_descriptor_sets_for_entity   (const size_t& entity_index);
    // see @file Agos/src/renderer/vulkan_app/vulkan_entity.h

    // ** images, image views, and buffers helpers
    // NOTE : all VulkanApp images, image views and buffers Helpers find their declarations and definitions to be in fileq Agos/src/renderer/vulkan_app/vulkan_app_helpers.h and .cpp

    // ** synchronisation objects (aka sync objects) helpers
    AgResult                    create_semaphore_objects();
    AgResult                    create_fence_objects();
    // ** ImGui layer
    void    imgui_draw_main_window();
    void    imgui_draw_viewport();
};  // * class VulkanApp


}   // * namespace VulkanHandler (within namespace Agos)

}   // * namespace Agos
