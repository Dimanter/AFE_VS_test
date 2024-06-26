#pragma once

#include "Service.hpp"

struct outService : public Service
{
    outService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv, sizeof(float) * 13 + sizeof(uint32_t) * 1 } {}

    BufferWrapper<float> outP = factory.create<float>(6000.0f);      ///< Амплитуда опорного сигнала (+)-канала (мВ)
    BufferWrapper<float> meanP = factory.create<float>(3000.0f);      ///< Среднее значение опорного сигнала (мВ)
    BufferWrapper<float> phaseP = factory.create<float>(0.0f);         ///< Фаза опорного сигнала (радиан)
    BufferWrapper<float> outN = factory.create<float>(6000.0f);      ///< Амплитуда опорного сигнала (-)-канала (мВ)
    BufferWrapper<float> meanN = factory.create<float>(3000.0f);      ///< Среднее значение опорного сигнала (мВ)
    BufferWrapper<float> phaseN = factory.create<float>(3.141592653f); ///< Фаза опорного сигнала (радиан)
    BufferWrapper<float> freq = factory.create<float>(400.f);        ///< Частота повторения.
};
