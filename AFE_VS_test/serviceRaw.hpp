#ifndef SERVICE_RAW_DATA_HPP
#define SERVICE_RAW_DATA_HPP

#include <initializer_list>

#include "Service.hpp"

template <typename T>
class serviceRAW : public Service
{
    using container_t = std::pmr::vector<T>;

public:
    using value_type = T;

    serviceRAW(eventCallback_t call, addressing_t serv, uint16_t size = 0) : Service(call, serv, static_cast<uint16_t>(sizeof(T)* size))
    {
        raw.reserve(size);
    }

    serviceRAW(eventCallback_t call, addressing_t serv, const std::initializer_list<T>& data) : Service(call, serv, static_cast<uint16_t>(sizeof(T)* data.size()))
    {
        raw = data;
    }

    serviceRAW(eventCallback_t call, addressing_t serv, const container_t& data) : Service(call, serv, static_cast<uint16_t>(sizeof(T)* data.size()))
    {
        raw = data;
    }

    serviceRAW& operator=(const std::initializer_list<T>& data)
    {
        raw = data;
        return *this;
    }

    serviceRAW& operator=(const container_t& data)
    {
        raw = data;
        return *this;
    }

    template <size_t N>
    serviceRAW& operator=(const T(*data)[N])
    {
        raw.clear();
        raw.reserve(N);
        std::copy(std::begin(*data), std::end(*data), raw.begin());
        return *this;
    }

    auto data()
    {
        return raw.data();
    }

    size_t size()
    {
        return raw.size();
    }

    size_t capacity()
    {
        return raw.capacity();
    }

    void resize(uint16_t size, const T& val = T{})
    {
        raw.reserve(size);
        raw.resize(size, val);
    }

    void reserve(uint16_t size)
    {
        raw.reserve(size);
    }

    auto& operator[](size_t idx)
    {
        return raw[idx];
    }

    template <typename... Args>
    void emplace_back(Args &&...args)
    {
        raw.emplace_back(std::forward<Args>(args)...);
    }

    void push_back(const T& val)
    {
        raw.push_back(val);
    }

    void pop_back()
    {
        raw.pop_back();
    }

    auto begin()
    {
        return raw.begin();
    }

    auto end()
    {
        return raw.end();
    }

    ~serviceRAW() = default;

private:
    container_t raw{ std::pmr::polymorphic_allocator{std::addressof(*buffer)} };
};

#endif // SERVICE_RAW_DATA_HPP
