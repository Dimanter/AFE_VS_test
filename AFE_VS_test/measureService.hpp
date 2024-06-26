#pragma once

#include "Service.hpp"

struct measure_t
{
	float RMS{ 0.f };
	float Bias{ 0.f };
	float Amp{ 0.f };
	float Phase{ 0.f };
};

struct measures_t
{
	measure_t V1;
	measure_t V2;
	measure_t I;
};

struct measureService : public Service
{
	measureService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv, sizeof(measures_t) + sizeof(float) } {}
	
	BufferWrapper<measures_t> Result = factory.create<measures_t>();
	BufferWrapper<float> refAngle = factory.create<float>(0);
};