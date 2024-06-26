#pragma once

#include "Service.hpp"

struct parameterService : public Service
{
    parameterService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv, sizeof(float) * 1 + sizeof(uint32_t) * 1 } {}

    BufferWrapper<float>    reference = factory.create<float>(0.5f);       ///< Отношение напряжение опорного источника смещения к питанию ADC
    BufferWrapper<uint32_t> encoder = factory.create<uint32_t>(3600000); ///< Количество отсчетов энкодера на оборот
};
