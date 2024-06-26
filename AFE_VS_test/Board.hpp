#pragma once

#include <QGraphicsView>
#include <QtWidgets>
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <math.h>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>
#include <QTextStream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <map>
#include <ranges>
#include <tuple>

#include "cyclebuff.hpp"
#include "measureService.hpp"

#include "device.hpp"
#include "qAxis.hpp"
#include "qTable.hpp"
#include "DataAnalyze.hpp"

#include <set>

class BoardGraphicsItem : public QGraphicsItem
{
    static constexpr std::size_t AVG = 175;
    static constexpr qreal       gap = 10;

    using Elem_t = float;
    using Point_t = Point<Elem_t>;
    using Container_t = std::set<Elem_t>;
    using pContainer_t = std::shared_ptr<Container_t>;

public:
    /**
     * @brief Конструктор с отрисовкой таблицы измерениями
     * @param width Ширина окна
     * @param height Высота окна
     */
    BoardGraphicsItem(qreal width, qreal height) : rectBounding(0, 0, width, height)
    {
        const auto fontSize = rectBounding.height() / 20;

        plotV1 = std::make_shared<Container_t>();
        plotVPhase1 = std::make_shared<Container_t>();
        plotI1 = std::make_shared<Container_t>();
        plotIPhase1 = std::make_shared<Container_t>();
        plotV2 = std::make_shared<Container_t>();
        plotVPhase2 = std::make_shared<Container_t>();


        tableError = new TableGraphicsItem(16, 2, width - 2 * gap, height / 8 - gap, "Error in linearity of the characteristic (min) 1.4.1 3.1.7");
        tableError->setParentItem(this);
        tableError->setToolTip("Deviations from the nominal voltage value should not exceed: \u00B116.8\'");
        tableError->setPos(gap, 6 * height / 8 - gap);

        tableResult = new TableGraphicsItem(10, 2, width - 2 * gap, height / 8 - gap, "Measurement results");
        tableResult->setParentItem(this);
        tableResult->setToolTip("Summary table of measurement results");
        tableResult->setPos(gap, 7 * height / 8);

        textRef = new QGraphicsTextItem(this);
        QFont font = textRef->font();
        font.setPixelSize(2 * fontSize);
        textRef->setFont(font);
        textRef->setDefaultTextColor(Qt::white);
        textRef->setPos(2 * gap, 2 * gap - fontSize);
        textRef->setTextWidth(2 * width / 5 - 2 * gap);

        textV1 = new QGraphicsTextItem(this);
        font = textV1->font();
        font.setPixelSize(fontSize);
        textV1->setFont(font);
        textV1->setDefaultTextColor(Qt::green);
        textV1->setPos(2 * width / 5 + gap, gap + fontSize);
        textV1->setTextWidth(width / 4 - gap);

        textV2 = new QGraphicsTextItem(this);
        font = textV2->font();
        font.setPixelSize(fontSize);
        textV2->setFont(font);
        textV2->setDefaultTextColor(Qt::yellow);
        textV2->setPos(3 * width / 5 + 2 * gap, gap + fontSize);
        textV2->setTextWidth(width / 4 - gap);

        textI1 = new QGraphicsTextItem(this);
        font = textI1->font();
        font.setPixelSize(fontSize);
        textI1->setFont(font);
        textI1->setDefaultTextColor(Qt::white);
        textI1->setPos(4 * width / 5 + 3 * gap, gap + fontSize);
        textI1->setTextWidth(width / 6 - gap);
    }

    /*@breif Метод удалаления элементов контенера выходящих за пределы угла
    * @param temp Контенер из которого нужно удалить элементы
    * @param minAngle Минимальный угол
    * @param maxAngle Максимальный угол
    */
    vector<Point_t> EraseForAngle(vector<Point_t> temp, float minAngle, float maxAngle)
    {
        vector<Point_t> res;

        std::for_each(temp.begin(), temp.end(), [&res,&minAngle,&maxAngle](const Point_t& el)
            {
                if (el.x() < minAngle || el.x() > maxAngle)
                {
                    res.push_back(el);
                }
            });

        return res;
    }

    void setParameters(float, const measures_t&);

    QRectF boundingRect() const override
    {
        return rectBounding;
    }
    /**
     * @brief Рисует таблицу каналов и тока
     */
    void paint(QPainter* pPainter, const QStyleOptionGraphicsItem*, QWidget*) override
    {
        pPainter->save();
        const auto fontSize = rectBounding.height() / 24;
        const auto width = (rectBounding.width() - 5 * gap) / 5;
        const auto height = (rectBounding.height() - 5 * gap) / 8;
        const auto top = rectBounding.y();
        const auto left = rectBounding.x();
        auto       font = pPainter->font();
        font.setPixelSize(fontSize);
        pPainter->setFont(font);
        pPainter->setPen(QPen(Qt::lightGray, 2));

        pPainter->drawRect(left + gap, top + gap, 2 * width, height);
        pPainter->drawText(left + 2 * width + 3 * gap, top + gap + fontSize, "Channel 1 (mV)");
        pPainter->drawRect(left + 2 * width + 2 * gap, top + gap, width, height);
        pPainter->drawText(left + 3 * width + 4 * gap, top + gap + fontSize, "Channel 2 (mV)");
        pPainter->drawRect(left + 3 * width + 3 * gap, top + gap, width, height);
        pPainter->drawText(left + 4 * width + 5 * gap, top + gap + fontSize, "Electrocution (mA)");
        pPainter->drawRect(left + 4 * width + 4 * gap, top + gap, width, height);

        pPainter->drawRect(left + gap, top + height + 2 * gap, rectBounding.width() - 2 * gap, 5 * height);

        pPainter->restore();
    }


    void Save();

    void Clear()
    {
        plotV1.reset();
        plotVPhase1.reset();
        plotV2.reset();
        plotVPhase2.reset();
        plotI1.reset();
        plotIPhase1.reset();
        update();
    }

    bool showInfo()
    {
        info.exec();
        if (QDialog::Accepted) {
            return true;
        }
        return false;
    }

    void Analyze();

    ~BoardGraphicsItem() = default;

private:
    const QRectF rectBounding;
    
    int counter = 0;

    std::vector<DataAnalyze> pV1;
    std::vector<DataAnalyze> pV2;
    std::vector<DataAnalyze> pVP1;
    std::vector<DataAnalyze> pVP2;
    std::vector<DataAnalyze> pI;
    std::vector<DataAnalyze> pIP;

    device info;
    std::vector<Point_t> plV1;
    std::vector<Point_t> plV2;
    std::vector<Point_t> plVP1;
    std::vector<Point_t> plVP2;

    QGraphicsTextItem* textRef;
    QGraphicsTextItem* textV1, * textVPhase1;
    QGraphicsTextItem* textV2, * textVPhase2;
    QGraphicsTextItem* textI1, * textIPhase1;

    pContainer_t plotV1, plotVPhase1;
    pContainer_t plotV2, plotVPhase2;
    pContainer_t plotI1, plotIPhase1;

    CycleBuffer<Elem_t> bufV1{ AVG }, bufVPhase1{ AVG };
    CycleBuffer<Elem_t> bufV2{ AVG }, bufVPhase2{ AVG };
    CycleBuffer<Elem_t> bufI1{ AVG }, bufIPhase1{ AVG };

    TableGraphicsItem* tableError, * tableResult;
};
