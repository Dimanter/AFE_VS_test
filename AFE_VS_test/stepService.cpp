#pragma once
#include <cstdint>

//#include "Service.hpp"

/**
 * @brief ���������� ����������� �������� ������.
 */
enum class stepDirection : uint32_t
{
    Forward = 0,
    Backward = 1
};

/**
 * @brief ���������� ��������� ����.
 */
enum class stepRatio : uint32_t
{
    _1 = 1,
    _1_2 = 2,
    _1_4 = 4,
    _1_8 = 8,
    _1_16 = 16,
    _1_32 = 32,
    _1_64 = 64,
    _1_128 = 128,
    _1_256 = 256
};

struct stepService //: public Service
{
    /*stepService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv, sizeof(uint32_t) * 4 } {}

    BufferWrapper<stepRatio>     ratio = factory.create<stepRatio>(stepRatio::_1_64);           ///< ������� ���, �����������
    BufferWrapper<uint32_t>      period = factory.create<uint32_t>(250);                         ///< ������ ��������� ��
    BufferWrapper<uint32_t>      step = factory.create<uint32_t>(350000);                      ///< ���������� ����� �� ������������� ����
    BufferWrapper<stepDirection> direction = factory.create<stepDirection>(stepDirection::Forward); ///< ����������� ��������*/
};
