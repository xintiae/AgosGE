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

#include "Agos/src/renderer/glfw_instance.h"
#include "Agos/src/renderer/vulkan_base/vulkan_base.h"
#include "Agos/src/renderer/vulkan_app/vulkan_modeling.h"
#include "Agos/src/renderer/modeling/model.h"

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
    VkSwapchainKHR              m_Swapchain;
    VkFormat                    m_SwapchainFormat;
    VkExtent2D                  m_SwapchainExtent;
    std::vector<VkImage>        m_SwapchainImages;
    std::vector<VkImageView>    m_SwapchainImageViews;
    std::vector<VkFramebuffer>  m_SwapchainFramebuffers;

    VkRenderPass                m_RenderPass;

    // m_GraphicsPipelineLayout m_GPL
    // m_GraphicsPipelineCache  m_GPC
    // m_GraphicsPipeline       m_GP
    // ? later on : m_ComputePipeline m_CP
    VkPipelineLayout            m_GPL_Lighting;
    VkPipelineCache             m_GPC_Lighting;
    VkPipeline                  m_GP_Lighting;

    VkPipelineLayout            m_GPL_Generic;
    VkPipelineCache             m_GPC_Generic;
    VkPipeline                  m_GP_Generic;


    // depth buffer
    VkImage                     m_DepthImage;
    VkDeviceMemory              m_DepthImageMemory;
    VkImageView                 m_DepthImageView;
    // color buffer for MSAA
    VkImage                     m_ColorImage;
    VkDeviceMemory              m_ColorImageMemory;
    VkImageView                 m_ColorImageView;

    

    // * Model Rendering
    std::vector<Agos::Modeling::Model>          m_ToRenderModels;
    // DescriptorSetLayout  DSL
    // DescriptorSet(s)     DS(s)
    VkDescriptorSetLayout                       m_DSL_MVP_EL;
    VkDescriptorSetLayout                       m_DSL_Material;
    VkDescriptorSetLayout                       m_DSL_TexAmbDiffSpecNrmlMaps;

    VkDescriptorPool                            m_DescriptorPool;

    // per model - per swap chain images descriptor sets
    std::vector<std::vector<VkDescriptorSet>>   m_DS_MVP_EL;
    std::vector<std::vector<VkDescriptorSet>>   m_DS_Material;
    std::vector<std::vector<VkDescriptorSet>>   m_DS_TexAmbDiffSpecNrmlMaps;


    // destructions tracking
    bool                        m_SwapchainDestroyed;               // = false
    bool                        m_SwapchainImagesDestroyed;         // = false
    bool                        m_SwapchainImageViewsDestroyed;     // = false
    bool                        m_SwapchainFramebuffersDestroyed;   // = false

    bool                        m_RenderPassDestroyed;              // = false
    bool                        m_GraphicsPipelineLayoutDestroyed;  // = false
    bool                        m_GraphicsPipelineCacheDestroyed;   // = false
    bool                        m_GraphicsPipelineDestroyed;        // = false

    bool                        m_ColorResDestroyed;                // = false
    bool                        m_DepthResDestroyed;                // = false

    bool                        m_DescriptorSetLayoutDestroyed;     // = false
    bool                        m_DescriptorPoolDestroyed;          // = false

    // true when all of previously enumerated m_xxxDestroyed have turned true
    bool                        m_VulkanAppTerminated;              // = false

public:
    explicit VulkanApp(std::shared_ptr<GLFWHandler::GLFWInstance>& glfw_instance);
    explicit VulkanApp(const VulkanApp& other)   = delete;
    explicit VulkanApp(VulkanApp&& other)        = delete;

    ~VulkanApp();
    VulkanApp& operator=(const VulkanApp& other)    = delete;
    VulkanApp& operator=(VulkanApp&& other)          = delete;

    AgResult    init_vulkan_app();
    AgResult    update_vulkan_app(const std::vector<Agos::Modeling::Model>& models_to_render);
    AgResult    terminate_vulkan_app();

protected:
    AgResult    recreate_swapchain();
    AgResult    terminate_swapchain(const bool& shall_recreate_swapchain_after = false);

private:

    //     AgResult    init_vulkan_app()
    //     {
    //          init_vulkan_base();
    AgResult    create_swapchain();
    AgResult    create_swapchain_image_views();
    AgResult    create_depth_res();
    AgResult    create_color_res();
    AgResult    create_render_pass();
    AgResult    create_swapchain_framebuffers();
    //     }
    //     AgResult    update_vulkan_app(const std::vector<VulkanModeling::Model>& models_to_render)
    //     {
    AgResult    create_descriptor_sets_layout();
    AgResult    create_graphics_pipelines();
    AgResult    create_descriptor_pool();
    AgResult    create_descriptor_sets();
    //     }

    //     AgResult    terminate_vulkan_app()
    //     {
    //         AgResult    terminate_swapchain(const bool& shall_recreate_swapchain_after = false)
    //         {
    AgResult    destroy_color_res               (const bool& shall_recreate_after = false);
    AgResult    destroy_depth_res               (const bool& shall_recreate_after = false);
    AgResult    destroy_swapchain_framebuffers  (const bool& shall_recreate_after = false);
    AgResult    destroy_graphics_pipelines      (const bool& shall_recreate_after = false);
    AgResult    destroy_render_pass             (const bool& shall_recreate_after = false);
    AgResult    destroy_swapchain               (const bool& shall_recreate_after = false);
    AgResult    destroy_descriptor_pool         (const bool& shall_recreate_after = false);
    //         }
    AgResult    destroy_decriptor_sets_layout();
    AgResult    destroy_swapchain_image_views();
    //          terminate_vulkan_base();
    //     }


    // ** = = = = = VulkanApp Helpers = = = = =
    // ** swapchain helper functions
    static VkSurfaceFormatKHR   choose_swap_surface_format  (const std::vector<VkSurfaceFormatKHR>&     available_formats       );
    static VkPresentModeKHR     choose_swap_present_mode    (const std::vector<VkPresentModeKHR>&       available_present_modes );
    static VkExtent2D           choose_swap_extent          (const VkSurfaceCapabilitiesKHR&            capabilities,
                                                             GLFWwindow*&                               glfw_instance           );
    // ** depth res helper functions
    static VkFormat             find_depth_format           (const VkPhysicalDevice&                    physical_device         );
    static VkFormat             find_supported_format       (const std::vector<VkFormat>&               candidates,
                                                             const VkImageTiling&                       tiling,
                                                             const VkFormatFeatureFlags&                features,
                                                             const VkPhysicalDevice&                    physical_device         );

    // ** graphics pipeline hepler functions
    // see @file Agos/src/renderer/vulkan_app/vulkan_app_helpers.h

    // ** descriptors hepler functions - uniform buffer objects for MVP matrices, lighting, materials, combined image samplers, etc...
    // see @file Agos/src/renderer/vulkan_app/vulkan_modeling.h

    // ** images, image views, and buffers helper functions
    // NOTE : all VulkanApp images, image views and buffers Helpers find their declarations and definitions to be in fileq Agos/src/renderer/vulkan_app/vulkan_app_helpers.h and .cpp
};  // * class VulkanApp


}   // * namespace VulkanHandler (within namespace Agos)

}   // * namespace Agos
