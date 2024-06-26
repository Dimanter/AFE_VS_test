#pragma once

#include <algorithm>
#include <numbers>
#include <numeric>
#include <type_traits>
#include <vector>

namespace DSP
{
    /**
     * @brief mean лямбда выражение подсчёт средного значения для указанного диапазона данных
     * @tparam IteratorType тип итератора
     * @tparam ElemT тип данных контейнера
     * @param begin начало диапазона
     * @param end конец диапазона
     * @return среднее значение
     */
    auto mean = []<typename IteratorType, typename ElemT = typename IteratorType::value_type>(IteratorType begin, IteratorType end)->ElemT
    {
        return std::accumulate(begin, end, ElemT{}) / (end - begin);
    };

    /**
     * @brief median лямбда выражение расчета серединного значения для указанного диапазона данных
     * @tparam IteratorType тип итератора
     * @tparam ElemT тип данных контейнера
     * @param begin начало диапазона
     * @param end конец диапазона
     * @return серединное значение
     */
    auto median = []<typename IteratorType, typename ElemT = typename IteratorType::value_type, typename Compare = std::less<ElemT>>(IteratorType begin, IteratorType end, Compare cmp = Compare{})->ElemT
    {
        std::vector<ElemT> res(end - begin);
        std::partial_sort_copy(begin, end, res.begin(), res.end(), cmp);
        return res[res.size() / 2];
    };

    /**
     * @brief max лямбда выражение расчета абсолютного максимального значения
     * @tparam IteratorType тип итератора
     * @tparam ElemT тип данных контейнера
     * @param begin начало диапазона
     * @param end конец диапазона
     * @return максимальное значение
     */
    auto max = []<typename IteratorType, typename ElemT = typename IteratorType::value_type>(IteratorType begin, IteratorType end)->ElemT
    {
        auto minmax = std::minmax_element(begin, end);
        if (*minmax.first * *minmax.first > *minmax.second * *minmax.second) 
        {
            return *minmax.first;
        }
        return *minmax.second;
    };


    /**
     * @brief smooth функция обработки данных с использованием алгоритма скользящего изменяемого окна
     * @tparam windowSize не типовой шаблонный параметр определяющий размер окна
     * @tparam ContainerType тип контейнера содержащего данные
     * @tparam Algo тип функтора определяющий алгоритм обработки данных внутри окна
     * @param cont ссылка на контейнер с входными данными
     * @return контейнер с обработанными данными
     */
    template <std::size_t windowSize, typename ContainerType, typename Algo>
    ContainerType smooth(const ContainerType& cont, Algo algo)
    {
        using ElemT = typename ContainerType::value_type;

        static_assert(windowSize % 2 != 0, "The window size must be odd number");

        const auto cbegin{ std::cbegin(cont) };
        const auto cend{ std::cend(cont) };

        ContainerType         res{ cont.size() };
        constexpr std::size_t halfWindow = (windowSize - 1) / 2;
        for (auto it = cbegin; it != cend; it++) 
        {
            auto start = it - halfWindow;
            auto stop = it + halfWindow;
            if (it - halfWindow < cbegin) 
            {
                start = cbegin;
                stop = it + (it - cbegin) + 1;
            }
            if (it + halfWindow >= cend) 
            {
                start = it - (cend - it) - 1;
                stop = cend;
            }
            res[it - cbegin] = algo(start, stop);
        }
        return res;
    }

    /**
     * @brief resmple функция обработки данных с использованием алгоритма передискретизации
     * @tparam windowSize не типовой шаблонный параметр определяющий размер окна
     * @tparam ContainerType тип контейнера содержащего данные
     * @tparam Algo тип функтора определяющий алгоритм обработки данных внутри окна
     * @param cont ссылка на контейнер с входными данными
     * @return контейнер с обработанными данными
     */
    template <std::size_t windowSize, typename ContainerType, typename Algo>
    ContainerType resample(const ContainerType& cont, Algo algo, ContainerType&& res = ContainerType{})
    {
        using ElemT = typename ContainerType::value_type;

        static_assert(std::is_arithmetic_v<ElemT>, "Container content must be numeric");

        const auto cbegin{ std::cbegin(cont) };
        const auto cend{ std::cend(cont) };

        auto it = cbegin;
        while (it < cend) 
        {
            auto start = it;
            auto stop = it + windowSize;
            if (stop > cend) 
            {
                stop = cend;
            }
            res.push_back(algo(start, stop));
            it += windowSize;
        }
        return std::move(res);
    }

    /**
     * @brief diff функция дифференцирования данных
     * @tparam ContainerType тип контейнера содержащего данные
     * @return контейнер с обработанными данными
     */
    template <std::size_t delta, typename ContainerType>
    auto diff(const ContainerType& cont)
    {
        if (cont.size() < 2 * delta) 
        {
            throw std::runtime_error("Container does not contain the required number of elements");
        }

        ContainerType res;

        res.reserve(cont.size() - 2 * delta);

        for (auto it = cont.begin() + delta; it < cont.end() - delta; it += delta) 
        {
            res.emplace_back(it->x(), ((it - delta)->y() - (it + delta)->y()) / ((it - delta)->x() - (it + delta)->x()));
        }
        return res;
    }

    template <std::size_t N, typename ContainerType>
    std::pair<ContainerType, ContainerType> findMinMax(const ContainerType& cont)
    {
        static constexpr float angleEpsilon{ 0.005 };

        auto iterators = [&cont](ContainerType::const_iterator iter) 
        {
            if (iter != cont.end()) 
            {
                auto start = iter - N * 100;
                if (start < cont.cbegin()) 
                {
                    start = cont.cbegin();
                }
                auto stop = iter + N * 100;
                if (start > cont.cend()) 
                {
                    start = cont.cend();
                }
                return std::make_pair(std::move(start), std::move(stop));
            }
            return std::make_pair(cont.cend(), cont.cend());
        };

        auto compY = [](const ContainerType::value_type& val1, const ContainerType::value_type& val2) { return val1.y() < val2.y(); };

        ContainerType mins, maxs;
        auto          tmp = smooth<N * 3>(diff<N * 15>(smooth<N * 3>(cont, median)), mean);
        for (auto it = tmp.begin() + 1; it < tmp.end(); ++it) 
        {
            if ((it - 1)->y() > 0 && it->y() < 0) 
            {
                maxs.emplace_back(it->x(), it->y());
            }
            else if ((it - 1)->y() < 0 && it->y() > 0) 
            {
                mins.emplace_back(it->x(), it->y());
            }
        }
        for (auto& point : mins) 
        {
            auto it = std::find_if(cont.cbegin(), cont.cend(), [&point](const ContainerType::value_type& val) { return std::abs(val.x() - point.x()) <= angleEpsilon; });

            auto [start, stop] = iterators(it);
            auto min = std::min_element(start, stop, compY);
            point.setX(min->x());
            point.setY(min->y());
        }
        for (auto& point : maxs) 
        {
            auto it = std::find_if(cont.cbegin(), cont.cend(), [&point](const ContainerType::value_type& val) { return std::abs(val.x() - point.x()) <= angleEpsilon; });

            auto [start, stop] = iterators(it);
            auto max = std::max_element(start, stop, compY);
            point.setX(max->x());
            point.setY(max->y());
        }
        return std::make_pair(std::move(mins), std::move(maxs));
    }

    /// @brief Выполнение операции над гармоническими сигналами
    /// @tparam T тип данных
    /// @tparam operate тип выполняемой операции
    /// @param Amp1 амплитуда первого сигнала
    /// @param Phase1 фаза первого сигнала
    /// @param Amp2 амплитуда второго сигнала
    /// @param Phase2 фаза второго сигнала
    /// @param op операция
    /// @return амплитуда результирующего сигнала
    template <typename T, typename operate = std::plus<T>>
    auto operateWaves(const T& Amp1, const T& Phase1, const T& Amp2, const T& Phase2, const operate& op = operate{}) -> T
    {
        return std::sqrt(op(Amp1 * Amp1 + Amp2 * Amp2, T{ 2 } *Amp1 * Amp2 * std::cos(Phase1 - Phase2))) / std::numbers::sqrt2_v<T>;
    }
}
