#pragma once

#include <type_traits>

template <class T>
concept arithmetic = std::is_floating_point_v<T> || std::is_integral_v<T>;

template <arithmetic T>
class Point
{
public:
    using value_type = T;

    Point() = default;
    Point(Point&&) = default;
    Point(const Point&) = default;

    Point(const T& X, const T& Y) : mX{ X }, mY{ Y } {}

    Point& operator=(Point&&) = default;
    Point& operator=(const Point&) = default;

    T x(void) const
    {
        return mX;
    }

    void setX(const T& val)
    {
        mX = val;
    }

    T y(void) const
    {
        return mY;
    }

    void setY(const T& val)
    {
        mY = val;
    }

    Point operator+(const Point& point) const
    {
        Point tmp{ *this };
        tmp += point;
        return tmp;
    }

    Point operator-(const Point& point) const
    {
        Point tmp{ *this };
        tmp -= point;
        return tmp;
    }

    template <arithmetic U>
    Point operator/(const U& val) const
    {
        Point tmp{ *this };
        tmp /= val;
        return tmp;
    }

    bool operator<(const Point& point) const noexcept
    {
        return mX < point.mX;
    }

    Point& operator+=(const Point& point)
    {
        mX += point.mX;
        mY += point.mY;
        return *this;
    }

    Point& operator-=(const Point& point)
    {
        mX -= point.mX;
        mY -= point.mY;
        return *this;
    }

    template <arithmetic U>
    Point& operator/=(const U& val)
    {
        mX /= val;
        mY /= val;
        return *this;
    }

    ~Point() = default;

private:
    T mX{};
    T mY{};
};

template <typename Stream, typename T>
Stream& operator<<(Stream& os, const Point<T>& point)
{
    os << point.x() << "\n"
        << point.y();
    return os;
}

namespace std
{
    template <>
    struct is_arithmetic<Point<float>> : public true_type
    {
    };

    template <>
    struct hash<Point<float>>
    {
        size_t operator()(const Point<float>& pt) const noexcept
        {
            return hash<float>{}(pt.x());
        }
    };
}
