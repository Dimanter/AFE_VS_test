#pragma once

#include "Service.hpp"

struct eventService : public Service
{
    enum Event : addressing_t
    {
        completePosition = 0,
        pressButton      = 1
    };

    eventService(Service::eventCallback_t call, addressing_t serv) : Service{call, serv, 1} {}

    BufferWrapper<addressing_t> event = factory.create<addressing_t>(0);
};
