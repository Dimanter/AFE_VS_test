#pragma once

#include <iostream>

#include "Service.hpp"

struct controlService : public Service
{
    enum Command : addressing_t
    {
        Load = 0,
        Save = 10,
        startPosition = 20,
        startCalibration = 30,
        startMonitor = 40,
        startMeasuremnt = 50,
        resetEncoder = 60,
        Stop = 70,
        Notify = 80,
        Colibrate = 90
    };

    enum Status : addressing_t
    {
        Idle = 0,
        Measuring = 1,
        Monitoring = 2,
        Positioning = 3
    };

    controlService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv, 2 } {}

    BufferWrapper<addressing_t> status = factory.create<addressing_t>(0);
    BufferWrapper<addressing_t> command = factory.create<addressing_t>(0);
};
