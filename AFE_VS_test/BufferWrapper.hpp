#pragma once

#include <cstring>

#include "BufferAllocator.hpp"

template <typename T> requires(std::is_arithmetic<T>::value == true)
T reverseByte(T&& val)
{
    union
    {
        T val;
        uint8_t buf[sizeof(T)];
    } res{ 0 };
    constexpr size_t size = sizeof(T);
    const T* const pVal = &val;
    for (size_t i = 0; i < size; ++i) {
        res.buf[size - i - 1] = *(reinterpret_cast<const uint8_t*>(pVal) + i);
    }
    return std::move(res.val);
}

template <typename T>
    requires(std::is_arithmetic<T>::value == false)
T reverseByte(T&& val)
{
    return std::forward<T>(val);
}

template <typename T>
class BufferWrapper final
{
    T orderByte(T&& val) const
    {
#ifdef LITTLE_ENDIAN
        return reverseByte(std::forward<T>(val));
#endif // LITTLE_ENDIAN x86(DEC) Arch / USB Stack

#ifdef BIG_ENDIAN
        return std::forward<T>(val);
#endif // BIG_ENDIAN ARM(IBM) Arch / IP Stack
        return std::forward<T>(val);
    }

public:
    BufferWrapper() = default;

    template <typename... Ts>
    BufferWrapper(BufferAllocator<T> alloc, Ts... args) : allocator{ alloc }
    {
        ptr = std::construct_at<T>(allocator.allocate(), orderByte(std::forward<Ts>(args))...);
    }

    BufferWrapper(const BufferWrapper& wrapper) = delete;

    BufferWrapper(BufferWrapper&& wrapper) noexcept : allocator{ std::move(wrapper.allocator) }
    {
        std::swap(ptr, wrapper.ptr);
    }

    BufferWrapper& operator=(BufferWrapper&& wrapper) noexcept
    {
        std::swap(*this, wrapper);
        return *this;
    }

    BufferWrapper& operator=(T&& val)
    {
        *ptr = orderByte(std::move(val));
        return *this;
    }

    BufferWrapper& operator=(const T& val)
    {
        operator=(T{ val });
        return *this;
    }

    operator T() { return orderByte(std::move(*ptr)); }

    T operator*()
    {
        return orderByte(std::move(*ptr));
    }

    ~BufferWrapper()
    {
        if (ptr != nullptr) {
            std::destroy_at(ptr);
            allocator.deallocate(ptr);
        }
    }

private:
    T* ptr{ nullptr };
    BufferAllocator<T> allocator;
};
