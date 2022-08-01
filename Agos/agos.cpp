#include "Agos/agos.h"
#define STB_IMAGE_IMPLEMENTATION


int main(int argc, char** argv)
{
    Agos::Application* AG_EDITOR = new(Agos::Application);
    // init loggers
    AG_EDITOR->init();
    AG_EDITOR->run();
    AG_EDITOR->terimnate();
    free(AG_EDITOR);
    return 0;
}