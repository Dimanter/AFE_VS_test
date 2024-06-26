#pragma once

#include <memory>

class Buffer;

class BufferIterator final
{
public:
    using value_type = unsigned char;

    BufferIterator(size_t p, std::shared_ptr<Buffer> buffer) : pos{ p }, buffer{ buffer } {}

    value_type& operator*();

    BufferIterator& operator++()
    {
        ++pos;
        return *this;
    }

    bool operator==(const BufferIterator& it) const
    {
        return it.pos == pos;
    }

    bool operator!=(const BufferIterator& it) const
    {
        return !this->operator==(it);
    }

private:
    size_t pos;
    Buffer* pBuffer() { return buffer.get(); }
    std::shared_ptr<Buffer> buffer;
};

namespace std
{
    template <>
    struct iterator_traits<BufferIterator>
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type = BufferIterator::value_type;
    };
}
