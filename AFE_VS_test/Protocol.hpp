#pragma once

#include <functional>

#include "Service.hpp"

enum class Services : addressing_t
{
    CoefsOut = 0U,
    CoefsIn1 = 10U,
    CoefsIn2 = 20U,
    Mode = 30U,
    Control = 40U,
    Event = 50U,
    Output = 60U,
    Step = 70U,
    Parameters = 80U,
    Reference1 = 90U,
    Reference2 = 100U,
    Measure1 = 110U,
    Measure2 = 120U,
    Measure3 = 130U
};

template <typename T>
using Callback_t = void (T::*)(const Service::serviceID_t&, const Service::State&);

template <typename T, typename Object, typename... Ts>
std::shared_ptr<T> makeService(Object* obj, Callback_t<Object> call, Services service, Ts &&...args)
{
    return std::make_shared<T>(std::bind(call, obj, static_cast<addressing_t>(service), std::placeholders::_1), static_cast<addressing_t>(service), std::forward<Ts>(args)...);
};
