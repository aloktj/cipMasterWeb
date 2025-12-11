#pragma once

#include "ConnectionLifecycleService.h"

class ConnectionLifecycleServiceProvider
{
public:
    static ConnectionLifecycleService *instance()
    {
        static ConnectionLifecycleService service;
        return &service;
    }
};

