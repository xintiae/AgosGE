#pragma once

#ifdef AG_GRAPHICS_API_VULKAN
#include "Agos/src/renderer/imgui/vulkan/vulkan_imgui.h"
#include "Agos/src/renderer/imgui/vulkan/vulkan_imgui_overlay.h"
#endif

#ifdef AG_GRAPHICS_API_OPENGL
#include "Agos/src/renderer/imgui/opengl/opengl_imgui.h"
#endif
