#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "./ui_mainwindow.h"

#include <functional>
#include <memory>

#include <QMainWindow>
#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>

#include "../AFE_VS_test/coefin.hpp"
#include "../AFE_VS_test/coefout.hpp"
#include "../AFE_VS_test/output.hpp"
#include "../AFE_VS_test/Protocol.hpp"

#include "../AFE_VS_test/Dispatcher.hpp"
#include "../AFE_VS_test/Process.hpp"
#include "../AFE_VS_test/coefOutService.hpp"
#include "../AFE_VS_test/controlService.hpp"
#include "../AFE_VS_test/eventService.hpp"
#include "../AFE_VS_test/measureService.hpp"
#include "../AFE_VS_test/parameterService.hpp"
#include "../AFE_VS_test/rawService.hpp"

#include "../AFE_VS_test/cyclebuf.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    static constexpr std::size_t AVG = 225;

    static constexpr std::size_t N = 360;

    Q_OBJECT

    float Frequency() const;

    auto load()
    {
        return [this]() {
            this->control->command = controlService::Load;
            this->control->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->control->getAddress(), Service::State::Ready); };
    }

    auto save()
    {
        return [this]() {
            this->control->command = controlService::Save;
            this->control->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->control->getAddress(), Service::State::Ready); };
    }

    auto sendCoefOut()
    {
        return [this]() {
            this->coefsOut->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsOut->getAddress(), Service::State::Ready); };
    }

    auto receiveCoefOut()
    {
        return [this]() {
            this->coefsOut->transmit(1, Service::Type::ReceiveConfirmed);
            return std::make_pair(this->coefsOut->getAddress(), Service::State::Ready); };
    }

    auto sendCoefIn1()
    {
        return [this]() {
            this->coefsIn1->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn1->getAddress(), Service::State::Ready); };
    }

    auto sendCoefIn2()
    {
        return [this]() {
            this->coefsIn2->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn2->getAddress(), Service::State::Ready); };
    }

    auto receiveCoefIn1()
    {
        return [this]() {
            this->coefsIn1->transmit(1, Service::Type::ReceiveConfirmed);
            return std::make_pair(this->coefsIn1->getAddress(), Service::State::Ready); };
    }

    auto receiveCoefIn2()
    {
        return [this]() {
            this->coefsIn2->transmit(1, Service::Type::ReceiveConfirmed);
            return std::make_pair(this->coefsIn2->getAddress(), Service::State::Ready); };
    }

    auto writeOutP(unsigned amp)
    {
        return [this, amp]() {
  /*          const float k = 2.f * std::numbers::pi_v<float> / static_cast<float>(N);
            for (size_t idx = 0; idx < N; ++idx) {
                OutP->raw[idx] = std::lround(amp * std::sin(k * idx) / 2.f + amp / 2.f);
            }
*/
          this->OutP->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->OutP->getAddress(), Service::State::Ready); };
    }

    auto writeOutN(unsigned amp)
    {
        return [this, amp]() {
 /*           const float k = 2.f * std::numbers::pi_v<float> / static_cast<float>(N);
            for (size_t idx = 0; idx < N; ++idx) {
                OutN->raw[idx] = std::lround(amp * std::sin(k * idx + std::numbers::pi_v<float>) / 2 + amp / 2);
            }
*/
           this->OutN->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->OutN->getAddress(), Service::State::Ready); };
    }

    auto setFreqOut()
    {
        return [this]() {
        this->out->freq = this->Frequency();
        this->out->transmit(1, Service::Type::TransmitConfirmed);
        return std::make_pair(this->out->getAddress(), Service::State::Ready); };
    }

    auto setGainOut(coefOutService::gain gain)
    {
        return [this, gain]() {
            this->coefsOut->CurrentGain = gain;
            this->coefsOut->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsOut->getAddress(), Service::State::Ready); };
    }

    auto setGainIn1(coefInService::gain gain)
    {
        return [this, gain]() {
            this->coefsIn1->CurrentGain = gain;
            this->coefsIn1->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn1->getAddress(), Service::State::Ready); };
    }

    auto setGainIn2(coefInService::gain gain)
    {
        return [this, gain]() {
            this->coefsIn2->CurrentGain = gain;
            this->coefsIn2->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn2->getAddress(), Service::State::Ready); };
    }

    auto startCalibration()
    {
        return [this]() {
            this->control->command = controlService::startCalibration;
            this->control->transmit(1, Service::Type::TransmitStreaming);
            return std::make_pair(this->control->getAddress(), Service::State::Ready); };
    }

    enum processID : Processes::processID_t
    {
        processSave = 0,
        processLoad,
        processOut_x1,
        processOut_x2,
        processOut_x4,
        processOut_x8,
        processIn_x1_8,
        processIn_x1_4,
        processIn_x1_2,
        processIn_x001,
        processIn_x002,
        processIn_x004,
        processIn_x008,
        processIn_x016,
        processIn_x032,
        processIn_x064,
        processIn_x128,
        flagMin   = 0b0100'0000,
        flagInCh1 = 0b1000'0000
    };

    void addProcessOut(processID proc, coefOutService::gain gain, unsigned min, unsigned max)
    {
        processes.add(proc | flagMin, writeOutN(min));
        processes.add(proc | flagMin, writeOutP(min));
        processes.add(proc | flagMin, setGainOut(gain));
        processes.add(proc | flagMin, setFreqOut());
        processes.add(proc | flagMin, startCalibration());
        processes.add(proc, writeOutN(max));
        processes.add(proc, writeOutP(max));
        processes.add(proc, setGainOut(gain));
        processes.add(proc, setFreqOut());
        processes.add(proc, startCalibration());
    }

    void addProcessIn(processID proc, coefInService::gain gainIn, coefOutService::gain gainOutMin, coefOutService::gain gainOutMax, unsigned min, unsigned max)
    {
        processes.add(proc | flagInCh1 | flagMin, writeOutN(min));
        processes.add(proc | flagInCh1 | flagMin, writeOutP(min));
        processes.add(proc | flagInCh1 | flagMin, setGainOut(gainOutMin));
        processes.add(proc | flagInCh1 | flagMin, setGainIn1(gainIn));
        processes.add(proc | flagInCh1 | flagMin, setFreqOut());
        processes.add(proc | flagInCh1 | flagMin, startCalibration());
        processes.add(proc | flagInCh1, writeOutN(max));
        processes.add(proc | flagInCh1, writeOutP(max));
        processes.add(proc | flagInCh1, setGainOut(gainOutMax));
        processes.add(proc | flagInCh1, setGainIn1(gainIn));
        processes.add(proc | flagInCh1, setFreqOut());
        processes.add(proc | flagInCh1, startCalibration());
        processes.add(proc | flagMin, writeOutN(min));
        processes.add(proc | flagMin, writeOutP(min));
        processes.add(proc | flagMin, setGainOut(gainOutMin));
        processes.add(proc | flagMin, setGainIn2(gainIn));
        processes.add(proc | flagMin, setFreqOut());
        processes.add(proc | flagMin, startCalibration());
        processes.add(proc, writeOutN(max));
        processes.add(proc, writeOutP(max));
        processes.add(proc, setGainOut(gainOutMax));
        processes.add(proc, setGainIn2(gainIn));
        processes.add(proc, setFreqOut());
        processes.add(proc, startCalibration());
    }

    uint8_t Write(std::unique_ptr<addressing_t[]>, std::size_t);
    void    showSetting();

    void eventProcess(const Processes::processID_t &);
    void notify(const Service::serviceID_t &, const Service::State &);
    void calIn(std::shared_ptr<coefInService>, char, float, float, float);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void readyRead();
    void portError(QSerialPort::SerialPortError);

    void on_btnRefresh_clicked();
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();

    void on_btnStart_clicked();
    void on_btnStop_clicked();

    void on_btnSave_clicked();
    void on_btnLoad_clicked();

    void on_btnFixIn_clicked();
    void on_btnCalIn_clicked();

    void on_btnFixOut_clicked();
    void on_btnCalOut_clicked();

    void on_lstOutGain_currentIndexChanged(int index);

    void on_lstInGain_currentIndexChanged(int index);

    void on_textLog_textChanged();

    void on_btnCoefIn1_clicked();
    void on_btnCoefIn2_clicked();

    void on_btnCoefOut_clicked();

private:
    float in1, in1_min, in1_max, in2, in2_min, in2_max, phase1, phase2;

    Processes                             processes{std::bind(&MainWindow::eventProcess, this, std::placeholders::_1)};
    Dispatcher                            dispatcher;
    std::shared_ptr<Transport>            transport = std::make_shared<Transport>(1, std::bind(&MainWindow::Write, this, _1, _2));
    std::shared_ptr<controlService>       control   = makeService<controlService>(this, &MainWindow::notify, Services::Control);
    std::shared_ptr<eventService>         message   = makeService<eventService>(this, &MainWindow::notify, Services::Event);
    std::shared_ptr<qOutput>              out       = makeService<qOutput>(this, &MainWindow::notify, Services::Output);
    std::shared_ptr<parameterService>     parameter = makeService<parameterService>(this, &MainWindow::notify, Services::Mode);
    std::shared_ptr<measureService>       measure   = makeService<measureService>(this, &MainWindow::notify, Services::Parameters);
    std::shared_ptr<qCoefOut>             coefsOut  = makeService<qCoefOut>(this, &MainWindow::notify, Services::CoefsOut);
    std::shared_ptr<qCoefIn>              coefsIn1  = makeService<qCoefIn>(this, &MainWindow::notify, Services::CoefsIn1);
    std::shared_ptr<qCoefIn>              coefsIn2  = makeService<qCoefIn>(this, &MainWindow::notify, Services::CoefsIn2);
    std::shared_ptr<rawService<uint32_t>> OutP      = makeService<rawService<uint32_t>>(this, &MainWindow::notify, Services::Reference1);
    std::shared_ptr<rawService<uint32_t>> OutN      = makeService<rawService<uint32_t>>(this, &MainWindow::notify, Services::Reference2);

    CycleBuffer<float> buf{AVG};

    QSerialPort *port;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
