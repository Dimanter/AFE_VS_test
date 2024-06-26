#pragma once

#include <iostream>

#include "Service.hpp"

struct coefInService : public Service
{
    enum gain : uint8_t// Коэфициент усиления
    {
        x1_8 = 0b00000111,
        x1_4 = 0b00001111,
        x1_2 = 0b00010111,
        x001 = 0b00011111,
        x002 = 0b00100111,
        x004 = 0b00101111,
        x008 = 0b00110111,
        x016 = 0b00111111,
        x032 = 0b01000111,
        x064 = 0b01001111,
        x128 = 0b01010111
    };

    static std::string strGain(const gain& gain)
    {
        switch (gain) {
        case x1_8:
            return "x1/8";
        case x1_4:
            return "x1/4";
        case x1_2:
            return "x1/2";
        case x001:
            return "x1";
        case x002:
            return "x2";
        case x004:
            return "x4";
        case x008:
            return "x8";
        case x016:
            return "x16";
        case x032:
            return "x32";
        case x064:
            return "x64";
        case x128:
            return "x128";
        }
        return "";
    }

    std::string strGain()
    {
        return strGain(CurrentGain);
    }

    coefInService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv, sizeof(float) * 33 + sizeof(gain) } {}

    auto data(void)
    {
        return buffer;
    }

    BufferWrapper<float> Volt_1_8_a = factory.create<float>(8.f);
    BufferWrapper<float> Volt_1_8_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_1_8 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_1_4_a = factory.create<float>(4.f);
    BufferWrapper<float> Volt_1_4_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_1_4 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_1_2_a = factory.create<float>(2.f);
    BufferWrapper<float> Volt_1_2_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_1_2 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_001_a = factory.create<float>(1.f);
    BufferWrapper<float> Volt_001_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_001 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_002_a = factory.create<float>(0.5f);
    BufferWrapper<float> Volt_002_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_002 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_004_a = factory.create<float>(0.25f);
    BufferWrapper<float> Volt_004_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_004 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_008_a = factory.create<float>(0.125f);
    BufferWrapper<float> Volt_008_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_008 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_016_a = factory.create<float>(0.0625f);
    BufferWrapper<float> Volt_016_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_016 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_032_a = factory.create<float>(0.03125f);
    BufferWrapper<float> Volt_032_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_032 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_064_a = factory.create<float>(0.015625f);
    BufferWrapper<float> Volt_064_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_064 = factory.create<float>(0.f);
    BufferWrapper<float> Volt_128_a = factory.create<float>(0.0078125f);
    BufferWrapper<float> Volt_128_b = factory.create<float>(0.f);
    BufferWrapper<float> Phase_128 = factory.create<float>(0.f);
    BufferWrapper<gain>  CurrentGain = factory.create<gain>(x1_8);
};
