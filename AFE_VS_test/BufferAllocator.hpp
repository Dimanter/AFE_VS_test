#pragma once

#include "Buffer.hpp"

#include <memory>

template <typename T>
struct BufferAllocator
{
public:
    using value_type = T;
    using pointer = value_type*;

    BufferAllocator()
    {
        buffer = std::make_shared<Buffer>();
    }

    BufferAllocator(std::shared_ptr<Buffer> buffer) : buffer{ buffer } {}

    BufferAllocator(const BufferAllocator& alloc) : buffer{ alloc.buffer } {}

    BufferAllocator(BufferAllocator&& alloc) : buffer{ std::move(alloc.buffer) } {}

    template <typename U>
    friend struct BufferAllocator;

    template <typename U>
    BufferAllocator(const BufferAllocator<U>& alloc) : buffer{ alloc.buffer } {}

    BufferAllocator& operator=(const BufferAllocator& alloc)
    {
        buffer = alloc.buffer;
        return *this;
    }

    pointer allocate(std::size_t n = 1)
    {
        return static_cast<pointer>(buffer->allocate(n * sizeof(value_type)));
    }

    void deallocate(pointer p, std::size_t n = 1)
    {
        buffer->deallocate(p, n);
    }

    void construct(pointer p, const T& t)
    {
        new (p) T(t);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

    struct rebind
    {
        typedef BufferAllocator other;
    };

    ~BufferAllocator() = default;

private:
    std::shared_ptr<Buffer> buffer;
};

template <typename T, typename U>
bool operator==(const BufferAllocator<T>& t, const BufferAllocator<U>& u) noexcept
{
    return t.buffer == u.buffer;
}

template <typename T, typename U>
bool operator!=(const BufferAllocator<T>& t, const BufferAllocator<U>& u) noexcept
{
    return !operator==<T, U>(t, u);
}
