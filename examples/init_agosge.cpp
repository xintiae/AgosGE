#include "Agos/agos.h"
#include <iostream>

class MyApplication : public Agos::AgApplication
{
public:
    MyApplication()
    {
    }

    ~MyApplication()
    {
    }

    // wanna set up things? do it here
    Agos::AgResult client_init_application()
    {
        // any based on Agos-API members (including logging system) has to be initialized BEFORE actually using them
        core_init_application();

        AG_CLIENT_WARN("Initializing client...");
        // put your init stuff here
        AG_CLIENT_INFO("Done initializing client!");

        return Agos::AG_SUCCESS;
    }

    // we loopin here
    Agos::AgResult client_run_application()
    {
        AG_CLIENT_WARN("Running client...");
        // core_run_application MAY and WILL surely evolve : its goal is to provide a link between user and core application
        // it consists of a continuus user-defined data sent to AgosGE for processing
        // so, e.g. you could draw a triangle and provide its vertices here

        core_run_application();

        return Agos::AG_SUCCESS;
    }

    Agos::AgResult client_terminate_application()
    {
        AG_CLIENT_WARN("Terminating client...");
        // got some bunch of members to clean? do it here :P
        // terminates and cleans up any based on Agos-API members APPART from LOGGING SYSTEM
        core_terminate_application();
        AG_CLIENT_INFO("Client termination successful!");
        return Agos::AG_SUCCESS;
    }
};

int main(int argc, char** argv)
{
    MyApplication* app = new MyApplication();
    app->client_init_application();
    app->client_run_application();
    app->client_terminate_application();
    delete(app);
    return 0;
}
