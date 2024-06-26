#ifndef TYPEDEF_HPP
#define TYPEDEF_HPP

#include <vector>

using addressing_t = uint8_t;

using Data_t = std::pmr::vector<addressing_t>;

enum class Status
{
    Success,
    Fail
};


#endif // TYPEDEF_HPP
