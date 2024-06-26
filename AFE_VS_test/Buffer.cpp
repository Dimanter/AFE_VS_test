#include "Buffer.hpp"

BufferIterator::value_type& BufferIterator::operator*()
{
    return (*pBuffer())[pos];
}
