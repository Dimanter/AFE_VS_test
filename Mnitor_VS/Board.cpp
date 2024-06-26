#include "Board.hpp"
#include "../AFE_VS_test/DSP.hpp"

void BoardGraphicsItem::setParameters(float angle, const measures_t& res)
{
    float grad, min, sec;
    grad = std::trunc(angle);
    min = angle - grad;
    min = std::trunc(min * 60.0);
    sec = (angle - grad) * 60.0 - min;
    sec = std::trunc(sec * 60.0);

    bufVFFT1.push(res.V1.Amp);
    plotV1->insert(Point_t{ angle, res.V1.Amp });
    bufVPhase1.push(res.V1.Phase);
    plotVPhase1->insert(Point_t{ angle, res.V1.Phase * 57.295779513082320876798154814105f });
    bufVRMS1.push(res.V1.RMS);
    plotVRMS1->insert(Point_t{ angle, res.V1.RMS });
    bufVBias1.push(res.V1.Bias);
    plotVBias1->insert(Point_t{ angle, res.V1.Bias });

    bufVFFT2.push(res.V2.Amp);
    plotVFFT2->insert(Point_t{ angle, res.V2.Amp });
    bufVPhase2.push(res.V2.Phase);
    plotVPhase2->insert(Point_t{ angle, res.V2.Phase * 57.295779513082320876798154814105f });
    bufVRMS2.push(res.V2.RMS);
    plotVRMS2->insert(Point_t{ angle, res.V2.RMS });
    bufVBias2.push(res.V2.Bias);
    plotVBias2->insert(Point_t{ angle, res.V2.Bias });

    bufIFFT.push(res.I.Amp);
    bufIPhase.push(res.I.Phase);
    bufIRMS.push(res.I.RMS);
    bufIBias.push(res.I.Bias);

    textRef->setHtml(QString("<big><bold>&alpha;</bold></big>: %1˚ %2\' %3\"").arg(static_cast<uint32_t>(grad), 3).arg(static_cast<uint32_t>(min), 2).arg(static_cast<uint32_t>(sec), 2));

    textIFFT->setHtml(QString("<bold>I</bold><sub> out</sub><small> (mA)</small>: %1").arg(bufIFFT.apply(DSP::mean), 5, 'f', 3));
    textIPhase->setHtml(QString("<bold>I</bold><sub> &phi;</sub>: %1˚").arg(bufIPhase.apply(DSP::median) * 57.295779513082320876798154814105, 5, 'f', 3));

    textIRMS->setHtml(QString("<bold>I</bold><sub><small>AC</small></sub><small> (mA)</small>: %1").arg(bufIRMS.apply(DSP::mean), 5, 'f', 3));
    textIBias->setHtml(QString("<bold>I</bold><sub><small>DC</small></sub><small> (mA)</small>: %1").arg(bufIBias.apply(DSP::mean), 5, 'f', 3));

    textVFFT1->setHtml(QString("<bold>U</bold><sub>1</sub><small> (mV)</small>: %1").arg(bufVFFT1.apply(DSP::mean), 5, 'f', 3));
    textVPhase1->setHtml(QString("<bold>U</bold><sub>&phi; 1</sub>: %1˚").arg(bufVPhase1.apply(DSP::median) * 57.295779513082320876798154814105, 5, 'f', 3));

    textVFFT2->setHtml(QString("<bold>U</bold><sub>2</sub><small> (mV)</small>: %1").arg(bufVFFT2.apply(DSP::mean), 5, 'f', 3));
    textVPhase2->setHtml(QString("<bold>U</bold><sub>&phi; 2</sub>: %1˚").arg(bufVPhase2.apply(DSP::median) * 57.295779513082320876798154814105, 5, 'f', 3));

    textVRMS1->setHtml(QString("<bold>U</bold><sub></sub><sub>1</sub><small> (mV)</small>: %1").arg(bufVRMS1.apply(DSP::mean), 5, 'f', 3));
    textVBias1->setHtml(QString("<bold>U</bold><sub>1</sub><sub><small> mean</small></sub><small> (mV)</small>: %1").arg(bufVBias1.apply(DSP::mean), 5, 'f', 3));

    textVRMS2->setHtml(QString("<bold>U</bold><sub>2</sub><small> (mV)</small>: %1").arg(bufVRMS2.apply(DSP::mean), 5, 'f', 3));
    textVBias2->setHtml(QString("<bold>U</bold><sub>2</sub><sub><small> mean</small></sub><small> (mV)</small>: %1").arg(bufVBias2.apply(DSP::mean), 5, 'f', 3));
}

void BoardGraphicsItem::Save()
{
    saveImpl("VRMS1.txt", plotVRMS1);
    saveImpl("VBias1.txt", plotVBias1);
    saveImpl("VFFT1.txt", plotV1);
    saveImpl("VPhase1.txt", plotVPhase1);
    saveImpl("VRMS2.txt", plotVRMS2);
    saveImpl("VBias2.txt", plotVBias2);
    saveImpl("VFFT2.txt", plotVFFT2);
    saveImpl("VPhase2.txt", plotVPhase2);
}
