#pragma once

#include "BufferIterator.hpp"

#include <algorithm>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <unordered_map>
#include <vector>

/**
 * @Brief Управление выделением памяти и хранение информации по выделеным
 * регионам
 */
class Buffer final : public std::enable_shared_from_this<Buffer>, public std::pmr::memory_resource
{
public:
    using value_type = unsigned char;
    using container_type = std::pmr::vector<value_type>;

    Buffer() { is_empty = true; }

    Buffer(std::size_t len) { buffer.reserve(len); }

    Buffer(const Buffer&) = delete;

    Buffer(Buffer&& buf) noexcept { std::swap(*this, buf); }

    Buffer& operator=(Buffer&& buf) noexcept
    {
        std::swap(*this, buf);
        return *this;
    }

    Buffer& operator=(const Buffer&) = delete;

    Buffer& operator=(const container_type& buf)
    {
        if (buf.size() == size()) {
            std::copy(buf.cbegin(), buf.cend(), buffer.begin());
        }
        else {
            throw std::invalid_argument("Buffer sizes are not equal, structure does not match");
        }
        return *this;
    }

    const value_type* const operator*() const { return buffer.data(); }

    value_type& operator[](std::size_t pos)
    {
        if (pos < buffer.size()) {
            return buffer[pos];
        }
        else {
            throw std::range_error("Index out of range");
        }
    }

    operator bool() { return !is_empty; }

    const container_type& data() const { return buffer; }

    bool operator==(const Buffer& buf) const
    {
        return buffer == buf.buffer && buffer.capacity() == buf.buffer.capacity() && metadata == buf.metadata;
    }

    bool operator!=(const Buffer& buf) const { return !operator==(buf); }

    void reserve(std::size_t len)
    {
        if (buffer.size() == 0) {
            buffer.reserve(len);
        }
        else {
            throw std::runtime_error("Will invalidate the pointer when reallocate on reserve");
        }
    }

    std::size_t size() const { return buffer.size(); }

    /**
     * @Brief создание итератора указывающего на начало
     * @return созданный итератор
     */
    BufferIterator begin() { return BufferIterator(offset, shared_from_this()); }

    BufferIterator end() { return BufferIterator(size(), shared_from_this()); }

protected:
    /// @brief Выделение памяти, без возможности реаллокации для сохранения
    /// валидности существующих указателей
    /// @param bytes
    /// @param alignment
    /// @return указатель на выделенный диапазон памяти в пределах буфера
    /// @exception В случае отсутствия необходимого свободного места
    void* do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        if (buffer.size() == 0) {
            reserve(bytes);
        }
        if (buffer.size() + bytes <= buffer.capacity()) {
            value_type* ptr = buffer.data() + buffer.size();
            metadata.emplace(buffer.size(), bytes);
            buffer.resize(buffer.size() + bytes);
            return ptr;
        }
        else {
            throw std::runtime_error("Not enough space, will invalidate the pointer when reallocate");
        }
    }

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override
    {
        auto it = metadata.find(static_cast<value_type*>(ptr) - buffer.data());
        if (it != metadata.end()) {
            const auto& [idx, len] = *it;
            if (idx + len == buffer.size() - offset) {
                buffer.erase(buffer.end() - len, buffer.end());
                metadata.erase(idx);
            }
            else if (idx <= offset) {
                buffer.erase(buffer.begin(), buffer.begin() + len);
                metadata.erase(idx);
                offset += len;
            }
            else {
                throw std::runtime_error("Buffer is fragmentation, will invalidate the pointer when moved");
            }
        }
        else {
            throw std::invalid_argument("Pointer is not valid");
        }
    }

    bool
        do_is_equal(const std::pmr::memory_resource& other) const noexcept override
    {
        return dynamic_cast<const std::pmr::memory_resource*>(this) == std::addressof(other);
    }

private:
    bool is_empty{ false };
    container_type buffer;
    std::size_t offset{ 0 };
    std::unordered_map<std::size_t, std::size_t> metadata;
};
