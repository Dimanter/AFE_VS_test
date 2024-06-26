#pragma once

#include <iostream>

#include "Service.hpp"

struct coefOutService : public Service
{
    enum gain : uint8_t
    {
        x1 = 0,
        x2 = 2,
        x4 = 3,
        x8 = 4
    };

    static std::string strGain(gain gain)
    {
        switch (gain) {
        case x1:
            return "x1";
        case x2:
            return "x2";
        case x4:
            return "x4";
        case x8:
            return "x8";
        }
        return "";
    }

    std::string strGain()
    {
        return strGain(CurrentGain);
    }

    coefOutService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv, sizeof(float) * 10 + sizeof(gain) } {}

    auto data(void)
    {
        return buffer;
    }

    BufferWrapper<float> Out_x1_a = factory.create<float>(0.572568f);
    BufferWrapper<float> Out_x1_b = factory.create<float>(-15.2034f);
    BufferWrapper<float> Out_x2_a = factory.create<float>(1.14518f);
    BufferWrapper<float> Out_x2_b = factory.create<float>(-30.6266f);
    BufferWrapper<float> Out_x4_a = factory.create<float>(2.29055f);
    BufferWrapper<float> Out_x4_b = factory.create<float>(-61.3632f);
    BufferWrapper<float> Out_x8_a = factory.create<float>(4.59751f);
    BufferWrapper<float> Out_x8_b = factory.create<float>(-138.699f);
    BufferWrapper<float> IOut = factory.create<float>(0.026356f);
    BufferWrapper<float> IPhase = factory.create<float>(0);
    BufferWrapper<gain>  CurrentGain = factory.create<gain>(x1); ///< Режим усиления выходного усилителя
};
