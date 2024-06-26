#ifndef QT_TABLE_HPP
#define QT_TABLE_HPP
#pragma once

#include <vector>
#include <exception>

#include <QtWidgets>
#include <QGraphicsView>

/**
 * @brief TableGraphicsItem класс отображения табличных жанных
 */
class TableGraphicsItem : public QGraphicsItem
{
public:
    TableGraphicsItem(size_t c, size_t r, qreal width, qreal height, QString n) : name{ n }, col{ c }, row{ r }, rectBounding{ 0, 0, width, height }
    {
        strings.resize(col * row);
    }

    std::pair<QString, QColor>& operator()(size_t c, size_t r)
    {
        if (c + r * col >= strings.size())
        {
            std::out_of_range("Index out of range in the cell table");
        }
        return strings[c + r * col];
    }

    void clear(void)
    {
        std::for_each(strings.begin(), strings.end(), [](std::pair<QString, QColor>& str)
            { str.first.clear(); });
    }

    QRectF boundingRect() const override
    {
        return rectBounding;
    }

    void paint(QPainter* pPainter, const QStyleOptionGraphicsItem*, QWidget*) override
    {
        const auto widthCol = rectBounding.width() / col;
        const auto heightRow = rectBounding.height() / (row + 0.5);
        const auto gapWidth = widthCol / 10;
        const auto gapHeight = heightRow / 10;

        pPainter->save();
        pPainter->setPen(QPen(Qt::white, 1));
        QFont font = pPainter->font();
        font.setPixelSize((heightRow - gapHeight) / 2);
        font.setFamily("Lucida Console");
        pPainter->setFont(font);
        pPainter->drawText(rectBounding.x(), rectBounding.y(), rectBounding.width(), heightRow / 2, Qt::AlignLeft, name);
        pPainter->setPen(QPen(Qt::gray, 2));
        pPainter->drawRect(rectBounding.x(), rectBounding.y() + heightRow / 2, rectBounding.width(), rectBounding.height() - heightRow / 2);
        pPainter->setPen(QPen(Qt::darkGray, 1));
        for (size_t i = 1; i < col; ++i)
        {
            pPainter->drawLine(rectBounding.x() + i * widthCol, rectBounding.y() + heightRow / 2, rectBounding.x() + i * widthCol, rectBounding.y() + rectBounding.height());
        }
        for (size_t i = 1; i < row; ++i)
        {
            pPainter->drawLine(rectBounding.x(), rectBounding.y() + heightRow / 2 + i * heightRow, rectBounding.width(), rectBounding.y() + heightRow / 2 + i * heightRow);
        }
        for (size_t i = 0; i < row; ++i)
        {
            for (size_t j = 0; j < col; ++j)
            {
                const auto& [str, color] = strings[j + i * col];
                pPainter->setPen(QPen(color, 1));
                pPainter->drawText(rectBounding.x() + gapWidth / 2 + j * widthCol, rectBounding.y() + (gapHeight + heightRow) / 2 + i * heightRow, widthCol - gapWidth, heightRow - gapHeight, Qt::AlignCenter, str);
            }
        }
        pPainter->restore();
    }

    ~TableGraphicsItem() = default;

private:
    std::vector<std::pair<QString, QColor>> strings;
    const QString name;
    const size_t col;
    const size_t row;
    const QRectF rectBounding;
};

#endif // QT_TABLE_HPP
