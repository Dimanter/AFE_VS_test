#ifndef CYCLE_BUFFER_HPP_
#define CYCLE_BUFFER_HPP_

#include <deque>

template <typename T>
requires(std::is_arithmetic<T>::value == true)
class CycleBuffer
{
public:
    CycleBuffer(std::size_t cnt) : count{cnt}
    {
        buffer.resize(count);
    }

    void clear(void)
    {
        buffer.clear();
    }

    void push(const T& item)
    {
        buffer.push_back(item);
        if (buffer.size() > count)
        {
            buffer.pop_front();
        }
    }

    template<typename Algo>
    T apply(Algo algo)
    {
        return algo(buffer.begin(), buffer.end());
    }

private:
    std::size_t count;
    std::deque<T> buffer;
};

#endif /* CYCLE_BUFFER_HPP_ */