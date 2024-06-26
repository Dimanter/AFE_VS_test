#pragma once

#include <QGraphicsView>
#include <QtWidgets>

#include "../AFE_VS_test/cyclebuff.hpp"
#include "../AFE_VS_test/measureService.hpp"
#include "../AFE_VS_test/qAxis.hpp"

#include <algorithm>
#include <set>

class BoardGraphicsItem : public QGraphicsItem
{
    static constexpr std::size_t AVG = 175;
    static constexpr qreal       gap = 10;

    using Elem_t = float;
    using Point_t = Point<Elem_t>;
    using Container_t = std::set<Point_t>;
    using pContainer_t = std::shared_ptr<Container_t>;

    void saveImpl(const QString& name, const pContainer_t& data)
    {
        QFile file(name);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream stream(&file);
            stream << data->size() << "\n";
            for (auto item : *data) {
                stream << item << "\n";
            }
            file.close();
        }
    }

public:
    BoardGraphicsItem(qreal width, qreal height) : rectBounding(0, 0, width, height)
    {
        plotV1 = std::make_shared<Container_t>();
        plotVPhase1 = std::make_shared<Container_t>();
        plotVRMS1 = std::make_shared<Container_t>();
        plotVBias1 = std::make_shared<Container_t>();
        plotVFFT2 = std::make_shared<Container_t>();
        plotVPhase2 = std::make_shared<Container_t>();
        plotVRMS2 = std::make_shared<Container_t>();
        plotVBias2 = std::make_shared<Container_t>();

        const auto fontSize = rectBounding.height() / 24;

        auto axisRMS = new AxisGraphicsItem(width / 2 - 6 * gap, 2 * (height - 6 * gap) / 5 - 5 * gap, 0, 360, 11000, 0, { {90, 3.5f, true}, {45, 3.0f, true}, {15, 2.5f, false} }, { {5000, 2.f, true}, {2500, 1.5f, true}, {500, 1.f, false} });
        axisRMS->setParentItem(this);
        axisRMS->setPos(3 * gap, (height - 6 * gap) / 5 + 3 * gap + fontSize);

        auto axisMean = new AxisGraphicsItem(width / 2 - 6 * gap, 2 * (height - 6 * gap) / 5 - 5 * gap, 0, 360, 1100, -1100, { {90, 3.5f, true}, {45, 3.0f, true}, {15, 2.5f, false} }, { {200, 2.f, true}, {100, 1.5f, false}, {50, 1.f, false} });
        axisMean->setParentItem(this);
        axisMean->setPos(3 * gap, 3 * (height - 6 * gap) / 5 + 4 * gap + fontSize / 2);

        auto axisFFT = new AxisGraphicsItem(width / 2 - 6 * gap, 2 * (height - 6 * gap) / 5 - 5 * gap, 0, 360, 11000, 0, { {90, 3.5f, true}, {45, 3.0f, true}, {15, 2.5f, false} }, { {5000, 2.f, true}, {2500, 1.5f, true}, {500, 1.f, false} });
        axisFFT->setParentItem(this);
        axisFFT->setPos((width - 3 * gap) / 2 + 4 * gap, (height - 6 * gap) / 5 + 3 * gap + fontSize);

        auto axisPhase = new AxisGraphicsItem(width / 2 - 6 * gap, 2 * (height - 6 * gap) / 5 - 5 * gap, 0, 360, 180, -180, { {90, 3.5f, true}, {45, 3.0f, true}, {15, 2.5f, false} }, { {90, 2.f, true}, {45, 1.5f, true}, {15, 1.f, false} });
        axisPhase->setParentItem(this);
        axisPhase->setPos((width - 3 * gap) / 2 + 4 * gap, 3 * (height - 6 * gap) / 5 + 4 * gap + fontSize / 2);

        axisRMS->addGraph(1, 1, plotVRMS1, Qt::green);
        axisRMS->addGraph(1, 1, plotVRMS2, Qt::yellow);
        axisMean->addGraph(1, 1, plotVBias1, Qt::green);
        axisMean->addGraph(1, 1, plotVBias2, Qt::yellow);

        axisFFT->addGraph(1, 1, plotV1, Qt::green);
        axisFFT->addGraph(1, 1, plotVFFT2, Qt::yellow);
        axisPhase->addGraph(1, 1, plotVPhase1, Qt::green);
        axisPhase->addGraph(1, 1, plotVPhase2, Qt::yellow);

        textRef = new QGraphicsTextItem(this);
        QFont font = textRef->font();
        font.setFamily("Agency FB");
        font.setPixelSize(2 * fontSize);
        textRef->setFont(font);
        textRef->setDefaultTextColor(Qt::white);
        textRef->setPos(2 * gap, 2 * gap);
        textRef->setTextWidth(width / 2 - gap);

        textIRMS = new QGraphicsTextItem(this);
        font = textIRMS->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textIRMS->setFont(font);
        textIRMS->setDefaultTextColor(Qt::white);
        textIRMS->setPos(width / 3 + 2 * gap, gap + fontSize);
        textIRMS->setTextWidth(width / 6 - gap);

        textIBias = new QGraphicsTextItem(this);
        font = textIBias->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textIBias->setFont(font);
        textIBias->setDefaultTextColor(Qt::white);
        textIBias->setPos(3 * width / 6 + 2 * gap, gap + fontSize);
        textIBias->setTextWidth(width / 6 - gap);

        textIFFT = new QGraphicsTextItem(this);
        font = textIFFT->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textIFFT->setFont(font);
        textIFFT->setDefaultTextColor(Qt::white);
        textIFFT->setPos(2 * width / 3 + 2 * gap, gap + fontSize);
        textIFFT->setTextWidth(width / 6 - gap);

        textIPhase = new QGraphicsTextItem(this);
        font = textIPhase->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textIPhase->setFont(font);
        textIPhase->setDefaultTextColor(Qt::white);
        textIPhase->setPos(5 * width / 6 + 2 * gap, gap + fontSize);
        textIPhase->setTextWidth(width / 6 - gap);

        textVRMS1 = new QGraphicsTextItem(this);
        font = textVRMS1->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVRMS1->setFont(font);
        textVRMS1->setDefaultTextColor(Qt::green);
        textVRMS1->setPos(12 * gap, (height - 6 * gap) / 5 + gap);
        textVRMS1->setTextWidth(width / 4 - gap);

        textVBias1 = new QGraphicsTextItem(this);
        font = textVBias1->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVBias1->setFont(font);
        textVBias1->setDefaultTextColor(Qt::green);
        textVBias1->setPos(12 * gap, 3 * (height - 6 * gap) / 5 + 3 * gap + fontSize / 2);
        textVBias1->setTextWidth(width / 4 - gap);

        textVRMS2 = new QGraphicsTextItem(this);
        font = textVRMS2->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVRMS2->setFont(font);
        textVRMS2->setDefaultTextColor(Qt::yellow);
        textVRMS2->setPos(12 * gap + width / 4, (height - 6 * gap) / 5 + gap);
        textVRMS2->setTextWidth(width / 4 - gap);

        textVBias2 = new QGraphicsTextItem(this);
        font = textVBias2->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVBias2->setFont(font);
        textVBias2->setDefaultTextColor(Qt::yellow);
        textVBias2->setPos(12 * gap + width / 4, 3 * (height - 6 * gap) / 5 + 3 * gap + fontSize / 2);
        textVBias2->setTextWidth(width / 4 - gap);

        textVFFT1 = new QGraphicsTextItem(this);
        font = textVFFT1->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVFFT1->setFont(font);
        textVFFT1->setDefaultTextColor(Qt::green);
        textVFFT1->setPos((width - 3 * gap) / 2 + 14 * gap, (height - 6 * gap) / 5 + gap);
        textVFFT1->setTextWidth(width / 4 - gap);

        textVPhase1 = new QGraphicsTextItem(this);
        font = textVPhase1->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVPhase1->setFont(font);
        textVPhase1->setDefaultTextColor(Qt::green);
        textVPhase1->setPos((width - 3 * gap) / 2 + 14 * gap, 3 * (height - 6 * gap) / 5 + 3 * gap + fontSize / 2);
        textVPhase1->setTextWidth(width / 4 - gap);

        textVFFT2 = new QGraphicsTextItem(this);
        font = textVFFT2->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVFFT2->setFont(font);
        textVFFT2->setDefaultTextColor(Qt::yellow);
        textVFFT2->setPos((width - 3 * gap) / 2 + 14 * gap + width / 4, (height - 6 * gap) / 5 + gap);
        textVFFT2->setTextWidth(width / 4 - gap);

        textVPhase2 = new QGraphicsTextItem(this);
        font = textVPhase2->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        textVPhase2->setFont(font);
        textVPhase2->setDefaultTextColor(Qt::yellow);
        textVPhase2->setPos((width - 3 * gap) / 2 + 14 * gap + width / 4, 3 * (height - 6 * gap) / 5 + 3 * gap + fontSize / 2);
        textVPhase2->setTextWidth(width / 4 - gap);
    }

    void setParameters(float, const measures_t&);

    QRectF boundingRect() const override
    {
        return rectBounding;
    }

    void paint(QPainter* pPainter, const QStyleOptionGraphicsItem*, QWidget*) override
    {
        pPainter->save();
        const auto fontSize = rectBounding.height() / 24;
        const auto width = (rectBounding.width() - 3 * gap) / 2;
        const auto height = (rectBounding.height() - 6 * gap) / 5;
        const auto top = rectBounding.y();
        const auto left = rectBounding.x();
        auto       font = pPainter->font();
        font.setFamily("Agency FB");
        font.setPixelSize(fontSize);
        pPainter->setFont(font);
        pPainter->setPen(QPen(Qt::lightGray, 2));

        pPainter->drawRect(left + gap, top + gap, 2 * width / 3 - gap, height);
        pPainter->drawText(left + 2 * width / 3 + 2 * gap, top + gap + fontSize, "RMS");
        pPainter->drawRect(left + 2 * width / 3 + gap, top + gap, 2 * width / 3, height);
        pPainter->drawText(left + 4 * width / 3 + 3 * gap, top + gap + fontSize, "FFT");
        pPainter->drawRect(left + 4 * width / 3 + 2 * gap, top + gap, 2 * width / 3, height);

        pPainter->drawRect(left + gap, top + height + 2 * gap, width, 4 * height + 3 * gap);
        pPainter->drawText(left + 2 * gap, top + height + 2 * gap + fontSize, "RMS");
        pPainter->drawRect(left + 2 * gap, top + height + 3 * gap + fontSize, width - 2 * gap, 2 * height - fontSize / 2);
        pPainter->drawRect(left + 2 * gap, top + 3 * height + 4 * gap + fontSize / 2, width - 2 * gap, 2 * height - fontSize / 2);

        pPainter->drawRect(left + width + 2 * gap, top + height + 2 * gap, width, 4 * height + 3 * gap);
        pPainter->drawText(left + width + 3 * gap, top + height + 2 * gap + fontSize, "FFT");
        pPainter->drawRect(left + width + 3 * gap, top + height + 3 * gap + fontSize, width - 2 * gap, 2 * height - fontSize / 2);
        pPainter->drawRect(left + width + 3 * gap, top + 3 * height + 4 * gap + fontSize / 2, width - 2 * gap, 2 * height - fontSize / 2);

        pPainter->restore();
    }

    void Save();

    void Clear()
    {
        plotV1->clear();
        plotVPhase1->clear();
        plotVFFT2->clear();
        plotVPhase2->clear();
        plotVRMS1->clear();
        plotVBias1->clear();
        plotVRMS2->clear();
        plotVBias2->clear();
        update();
    }

    ~BoardGraphicsItem() = default;

private:
    const QRectF rectBounding;

    QGraphicsTextItem* textRef;
    QGraphicsTextItem* textIFFT, * textIPhase, * textIRMS, * textIBias;
    QGraphicsTextItem* textVFFT1, * textVPhase1, * textVRMS1, * textVBias1;
    QGraphicsTextItem* textVFFT2, * textVPhase2, * textVRMS2, * textVBias2;

    pContainer_t plotV1, plotVPhase1, plotVRMS1, plotVBias1;
    pContainer_t plotVFFT2, plotVPhase2, plotVRMS2, plotVBias2;

    CycleBuffer<Elem_t> bufVFFT1{ AVG }, bufVPhase1{ AVG }, bufVRMS1{ AVG }, bufVBias1{ AVG };
    CycleBuffer<Elem_t> bufVFFT2{ AVG }, bufVPhase2{ AVG }, bufVRMS2{ AVG }, bufVBias2{ AVG };
    CycleBuffer<Elem_t> bufIFFT{ AVG }, bufIPhase{ AVG }, bufIRMS{ AVG }, bufIBias{ AVG };
};
