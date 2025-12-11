#include <drogon/drogon.h>
#include "controllers/HealthController.h"

int main(int argc, char *argv[])
{
    drogon::app().loadConfigFile("config/config.json");
    drogon::app().run();
    return 0;
}
