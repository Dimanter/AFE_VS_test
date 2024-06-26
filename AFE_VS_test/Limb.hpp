#ifndef LIMB_H
#define LIMB_H
#pragma once

#include <initializer_list>
#include <optional>
#include <vector>
#include <algorithm>
#include <tuple>

/**
 * @brief Класс градуирования шкалы
 * @tparam NumT тип индекса (шкалы)
 * @tparam ValT тип значения отметки на шкале
 */
template <typename IdxT, typename ValT>
class Limb
{
    using ElemT = typename std::tuple<IdxT, ValT, bool>;
public:
    using args = std::initializer_list<ElemT>;

    Limb() = delete;

    /**
     * @brief Limb конструктор
     * @param list список инициализации кортеж значений: индекс, величина отметки, признак отображения сетки и значения на шкале
     * @details возвращается первое совпавшее(кратное) значение, указываются кортеж значений от более к менее значимым отметкам
     */
    Limb(args list) : items{ list } {}

    std::optional<std::pair<ValT, bool>> getValue(const IdxT& idx) const
    {
        for (const auto& item : items) {
            const size_t rational = idx / std::get<0>(item);
            if ((idx < std::get<0>(item) * rational + 1E-5) && (idx > std::get<0>(item) * rational - 1E-5)) {
                return std::make_pair(std::get<1>(item), std::get<2>(item));
            }
        }
        return std::nullopt;
    }

    ValT getMin() const
    {
        return std::get<0>(*std::min_element(items.cbegin(), items.cend(), [](const ElemT& a, const ElemT& b) { return std::get<0>(a) < std::get<0>(b); }));
    }

    ~Limb() = default;


private:
    const std::vector<ElemT> items{};
};

#endif // LIMB_H
