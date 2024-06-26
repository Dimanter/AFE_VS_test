#pragma once

#include "Service.hpp"

template <typename T>
struct rawService : public Service
{
    using value_type = T;
    using container_t = std::pmr::vector<value_type>;

    rawService(Service::eventCallback_t call, addressing_t serv) : Service{ call, serv } {}

    template <size_t N>
    rawService& operator=(const T(*data)[N])
    {
        raw.clear();
        raw.reserve(N);
        std::copy(std::begin(*data), std::end(*data), raw.begin());
        return *this;
    }

    container_t raw{ std::pmr::polymorphic_allocator{std::addressof(*buffer)} };
};
