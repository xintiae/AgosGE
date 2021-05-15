#include "Agos/agos.h"
#include "Agos/src/logger.h"
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

    Agos::AgResult client_init_application()
    {
        core_init_application();    // any based Agos-API members (including logging system) has to be initialized

        AG_CLIENT_WARN("Initializing client...");
        AG_CLIENT_INFO("Done initializing client!");

        return Agos::AG_SUCCESS;
    }

    Agos::AgResult client_run_application()
    {
        AG_CLIENT_WARN("Running client...");
        core_run_application();                 // mandatory
        return Agos::AG_SUCCESS;
    }

    Agos::AgResult client_terminate_application()
    {
        AG_CLIENT_WARN("Terminating client...");
        core_terminate_application();           // mandatory
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