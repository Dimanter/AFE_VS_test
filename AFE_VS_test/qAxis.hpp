#ifndef QT_AXIS_HPP
#define QT_AXIS_HPP
#pragma once

#include <cmath>
#include <set>

#include <QtWidgets>
#include <QGraphicsView>

#include "Point.hpp"
#include "Limb.hpp"

/**
 * @brief AxisGraphicsItem класс отображения осей координат и графика зависимости
 */
class AxisGraphicsItem : public QGraphicsItem
{
public:
    /**
     * @brief конструктор
     * @param w - ширина оси X в графических единицах
     * @param h - высота оси Y в графических единицах
     * @param l - левая граница графика
     * @param r - правая граница графика
     * @param t - верхняя граница графика
     * @param b - нижняя граница графика
     * @param listX - список инициализации определяющий деления для оси X, представляет кортеж чисел кратность делений и размер деления и признак отображения сетки(числвого значения)
     * @param listY - список инициализации определяющий деления для оси Y, представляет кортеж чисел кратность делений и размер деления и признак отображения сетки(числвого значения)
     */
    AxisGraphicsItem(const qreal w, const qreal h, const int l, const int r, const int t, const int b, Limb<float, float>::args listX, Limb<float, float>::args listY) : width{ w }, height{ h }, left{ l }, right{ r }, top{ t }, bottom{ b }, limbX{ listX }, limbY{ listY } {}

    /**
     * @brief метод подключения графика
     * @param scaleX - масштабный коэффициент (множитель) по оси X
     * @param scaleY - масштабный коэффициент (множитель) по оси Y
     * @param graph - указатель на контейнер с данными для графика
     */
    void addGraph(float scaleX, float scaleY, std::shared_ptr<std::set<Point<float>>> graph, QColor color = Qt::green)
    {
        color.setAlpha(127);
        graphs.emplace_back(scaleX, scaleY, graph, color);
    }

    QRectF boundingRect() const override
    {
        return QRectF(0, 0, width, height);
    }

    void drawAxis(QPainter* pPainter)
    {
        pPainter->save();
        pPainter->setPen(QPen(Qt::yellow, 2));
        const float mulX = static_cast<float>(width) / static_cast<float>(right - left);
        const float mulY = static_cast<float>(height) / static_cast<float>(top - bottom);
        if (bottom >= 0)
            pPainter->drawLine(0, height, width, height);
        if (top <= 0)
            pPainter->drawLine(0, 0, width, 0);
        else
            pPainter->drawLine(0, height + static_cast<qreal>(bottom * mulY), width, height + static_cast<qreal>(bottom * mulY));
        if (left >= 0)
            pPainter->drawLine(0, height, 0, 0);
        if (right <= 0)
            pPainter->drawLine(width, height, width, 0);
        else
            pPainter->drawLine(width - static_cast<qreal>(right * mulX), 0, width - static_cast<qreal>(right * mulX), height);
        pPainter->restore();
    }

    void drawScale(QPainter* pPainter)
    {
        pPainter->save();
        QFont font = pPainter->font();
        font.setFamily("Agency FB");
        pPainter->setFont(font);
        pPainter->setPen(QPen(Qt::white, 1));
        const float mulX = static_cast<float>(width) / static_cast<float>(right - left);
        const float mulY = static_cast<float>(height) / static_cast<float>(top - bottom);
        auto stepX = limbX.getMin();
        decltype(stepX) i = left + stepX;
        while (i < right)
        {
            if (i != 0)
            {
                if (const auto& add = limbX.getValue(std::abs(i)))
                {
                    const qreal x_i = static_cast<qreal>(mulX * (i - left));
                    const qreal delta = static_cast<qreal>(add.value().first * static_cast<float>(height) / 100.f);
                    qreal y = height + static_cast<qreal>(mulY * bottom);
                    if (bottom >= 0)
                        pPainter->drawLine(x_i, height, x_i, height - delta);
                    if (top <= 0)
                        pPainter->drawLine(x_i, 0, x_i, delta);
                    else
                        pPainter->drawLine(x_i, y, x_i, y - delta);
                    if (add.value().second)
                    {
                        QFont font = pPainter->font();
                        font.setPixelSize(delta);
                        pPainter->setFont(font);
                        if (bottom >= 0)
                            pPainter->drawText(x_i - delta, height - 2 * delta, 2 * delta, delta, Qt::AlignCenter, QString::number(i));
                        if (top <= 0)
                            pPainter->drawText(x_i - delta, 0 + delta, 2 * delta, delta, Qt::AlignCenter, QString::number(i));
                        else
                            pPainter->drawText(x_i - delta, y - 2 * delta, 2 * delta, delta, Qt::AlignCenter, QString::number(i));
                    }
                }
            }
            i += stepX;
        }
        auto stepY = limbY.getMin();
        decltype(stepY) j = bottom + stepY;
        while (j < top)
        {
            if (j != 0)
                if (const auto& add = limbY.getValue(std::abs(j)))
                {
                    const qreal y_i = height - static_cast<qreal>(mulY * (j - bottom));
                    const qreal delta = static_cast<qreal>(add.value().first * static_cast<float>(width) / 100.f);
                    qreal x = width - static_cast<qreal>(mulX * right);
                    if (left >= 0)
                        pPainter->drawLine(0, y_i, delta, y_i);
                    if (right <= 0)
                        pPainter->drawLine(width, y_i, width - delta, y_i);
                    else
                        pPainter->drawLine(x, y_i, x + delta, y_i);
                    if (add.value().second)
                    {
                        QFont font = pPainter->font();
                        font.setPixelSize(delta);
                        pPainter->setFont(font);
                        if (left >= 0)
                            pPainter->drawText(delta, y_i - delta / 2, 2 * delta, delta, Qt::AlignCenter, QString::number(j));
                        if (right <= 0)
                            pPainter->drawText(width - delta, y_i - delta / 2, 2 * delta, delta, Qt::AlignCenter, QString::number(j));
                        else
                            pPainter->drawText(x + delta, y_i - delta / 2, 2 * delta, delta, Qt::AlignCenter, QString::number(j));
                    }
                }
            j += stepY;
        }
        pPainter->restore();
    }

    void drawGrid(QPainter* pPainter)
    {
        pPainter->save();
        pPainter->setPen(QPen(Qt::darkGray, 0.7));
        const float mulX = static_cast<float>(width) / static_cast<float>(right - left);
        const float mulY = static_cast<float>(height) / static_cast<float>(top - bottom);
        auto stepX = limbX.getMin();
        decltype(stepX) i = left + stepX;
        while (i < right)
        {
            if (const auto& add = limbX.getValue(std::abs(i)))
            {
                if (add.value().second && i != 0)
                {
                    const qreal x_i = static_cast<qreal>(mulX * (i - left));
                    const qreal deltaX = static_cast<qreal>(add.value().first * static_cast<float>(width) / 1000.f);
                    auto stepY = limbY.getMin();
                    decltype(stepY) j = bottom + stepY;
                    while (j < top)
                    {
                        if (const auto& add = limbY.getValue(std::abs(j)))
                            if (add.value().second && j != 0)
                            {
                                const qreal y_i = height - static_cast<qreal>(mulY * (j - bottom));
                                const qreal deltaY = static_cast<qreal>(add.value().first * static_cast<float>(width) / 1000.f);
                                const qreal delta = min(deltaY, deltaX);
                                pPainter->drawLine(x_i, y_i - delta, x_i, y_i + delta);
                                pPainter->drawLine(x_i - delta, y_i, x_i + delta, y_i);
                            }
                        j += stepY;
                    }
                }
            }
            i += stepX;
        }
        pPainter->restore();
    }

    void drawGraph(QPainter* pPainter)
    {
        pPainter->save();
        const float mulX = static_cast<float>(width) / static_cast<float>(right - left);
        const float mulY = static_cast<float>(height) / static_cast<float>(top - bottom);
        for (const auto& graph : graphs)
        {
            pPainter->setPen(QPen(std::get<3>(graph), 1));
            const float scaleX{ std::get<0>(graph) };
            const float scaleY{ std::get<1>(graph) };
            for (auto& point : *(std::get<2>(graph)))
            {
                const qreal x = (point.x() * scaleX - left) * mulX;
                const qreal y = height - point.y() * scaleY * mulY + static_cast<qreal>(mulY * bottom);
                pPainter->drawPoint(x, y);
            }
        }
        pPainter->restore();
    }

    void paint(QPainter* pPainter, const QStyleOptionGraphicsItem*, QWidget*) override
    {
        pPainter->save();
        drawGrid(pPainter);
        drawScale(pPainter);
        drawAxis(pPainter);
        drawGraph(pPainter);
        pPainter->restore();
    }

    ~AxisGraphicsItem() = default;

private:
    std::vector<std::tuple<float, float, std::shared_ptr<std::set<Point<float>>>, QColor>> graphs;
    const qreal width, height;
    const int left, right, top, bottom;
    const Limb<float, float> limbX, limbY;
};

#endif // QT_AXIS_HPP
