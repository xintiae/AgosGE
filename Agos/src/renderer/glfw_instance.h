#pragma once

#include "Agos/src/core.h"
#include "Agos/src/base.h"

#include <GLFW/glfw3.h>

namespace Agos
{
typedef class AG_API AgGLFWHandler
{
private:
    GLFWwindow* m_ApplicationWindow;
    bool m_Terminated = false;

public:
    AgGLFWHandler();
    ~AgGLFWHandler();

    AgResult init();
    AgResult terminate();

    GLFWwindow*& get_window();
} AgGLFWHandler;

} // namespace Agos
