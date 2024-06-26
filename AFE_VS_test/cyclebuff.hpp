#pragma once 

#include <iostream>
#include <concepts>
#include <deque>

using namespace std;
//@brief CycleBuffer Класс циклического буфера
template <typename T> class CycleBuffer
{
public:
	CycleBuffer(size_t cnt) : count{ cnt }
	{
		buffer.resize(count);
	}

	void clear()
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
	size_t count;
	deque<T> buffer;
};