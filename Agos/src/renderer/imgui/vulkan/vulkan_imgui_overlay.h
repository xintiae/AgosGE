#pragma once

#include "Agos/src/base.h"
#include "Agos/src/core.h"

namespace Agos
{
namespace ImGuiHandler
{
namespace ImGuiVulkan
{

// ~ WIP
struct Overlay
{
    static void window();
    //          {
    static void     menu_bar();
    //              {
    static void draw_main_window();
    static void draw_viewport();

    static void draw_main_menu_bar();

    static void append_menu_bar();
};

}   // namespace ImGuiVulkan (within namespace Agos::ImGuiHandler)
}   // namespace ImGuiHandler (within namespace Agos)
}   // namespace Agos