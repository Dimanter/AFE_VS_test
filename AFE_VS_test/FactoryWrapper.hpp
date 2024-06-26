#pragma once

#include "BufferWrapper.hpp"

class FactoryWrapper final
{
public:
    FactoryWrapper(std::shared_ptr<Buffer> buffer) : buffer{ buffer } {}

    template <typename T, typename... Ts>
    BufferWrapper<T> create(Ts... Args)
    {
        return BufferWrapper<T>{BufferAllocator<T>{buffer}, std::forward<Ts>(Args)...};
    }

private:
    std::shared_ptr<Buffer> buffer;
};
