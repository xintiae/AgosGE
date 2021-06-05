#include "debug_layers.h"

Agos::AgResult Agos::ag_init_debug_layers()
{
    Agos::ag_vulkan_setup_debug_messenger();
    return Agos::AG_SUCCESS;
}
