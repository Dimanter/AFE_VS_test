#pragma once

#include <functional>
#include <memory>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <math.h>

#include "../AFE_VS_test/coefin.hpp"
#include "../AFE_VS_test/coefout.hpp"
#include "../AFE_VS_test/output.hpp"
#include "../AFE_VS_test/protocol.hpp"
#include "../AFE_VS_test/Dispatcher.hpp"
#include "../AFE_VS_test/Process.hpp"
#include "../AFE_VS_test/coefOutService.hpp"
#include "../AFE_VS_test/controlService.hpp"
#include "../AFE_VS_test/eventService.hpp"
#include "../AFE_VS_test/measureService.hpp"
#include "../AFE_VS_test/parameterService.hpp"
#include "../AFE_VS_test/rawService.hpp"
#include "../AFE_VS_test/cyclebuff.hpp"

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
public:
    //запись
    uint8_t Write(std::unique_ptr<addressing_t[]>, std::size_t);
private:
    enum processID : Processes::processID_t
    {
        processSave = 0,//процесс сохранения
        processLoad,//процесс загрузки
        processOut_x1,//процесс усиления выхода на 1
        processOut_x2,//процесс усиления выхода на 2
        processOut_x4,//процесс усиления выхода на 4
        processOut_x8,//процесс усиления выхода на 8
        processIn_x1_8,//процесс усиления входа на 1/8
        processIn_x1_4,//процесс усиления входа на 1/4
        processIn_x1_2,//процесс усиления входа на 1/2
        processIn_x001,//процесс усиления входа на 1
        processIn_x002,//процесс усиления входа на 2
        processIn_x004,//процесс усиления входа на 4
        processIn_x008,//процесс усиления входа на 8
        processIn_x016,//процесс усиления входа на 16
        processIn_x032,//процесс усиления входа на 32
        processIn_x064,//процесс усиления входа на 64
        processIn_x128,//процесс усиления входа на 128
        flagMin = 0b0100'0000,//флаг минимума
        flagInCh1 = 0b1000'0000//флаг 1 канала
    };
    //настройки(не работает)
    void    showSetting();

    void eventProcess(const Processes::processID_t&);
    //отслеживание работы процессов
    void notify(const Service::serviceID_t&, const Service::State&);
    //Калибровка входа
    void calIn(std::shared_ptr<coefInService>, char, float, float, float);

    static constexpr std::size_t AVG = 225;

    static constexpr std::size_t N = 360;

    Q_OBJECT
        float Frequency() const;
    //загрузить калибровку
    auto load()
    {
        return [this]() {
            this->control->command = controlService::Load;
            this->control->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->control->getAddress(), Service::State::Ready); };
    }
    //сохранить калибровку
    auto save()
    {
        return [this]() {
            this->control->command = controlService::Save;
            this->control->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->control->getAddress(), Service::State::Ready); };
    }
    //отправить данные на выходной канал
    auto sendCoefOut()
    {
        return [this]() {
            this->coefsOut->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsOut->getAddress(), Service::State::Ready); };
    }
    //считать данные с выходного канала
    auto receiveCoefOut()
    {
        return [this]() {
            this->coefsOut->transmit(1, Service::Type::ReceiveConfirmed);
            return std::make_pair(this->coefsOut->getAddress(), Service::State::Ready); };
    }
    //отправить данные на первый входной канал
    auto sendCoefIn1()
    {
        return [this]() {
            this->coefsIn1->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn1->getAddress(), Service::State::Ready); };
    }
    //отправить данные на второй входной канал
    auto sendCoefIn2()
    {
        return [this]() {
            this->coefsIn2->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn2->getAddress(), Service::State::Ready); };
    }
    //считать данные с первого входного канала
    auto receiveCoefIn1()
    {
        return [this]() {
            this->coefsIn1->transmit(1, Service::Type::ReceiveConfirmed);
            return std::make_pair(this->coefsIn1->getAddress(), Service::State::Ready); };
    }
    //считать данные со второго входного канала
    auto receiveCoefIn2()
    {
        return [this]() {
            this->coefsIn2->transmit(1, Service::Type::ReceiveConfirmed);
            return std::make_pair(this->coefsIn2->getAddress(), Service::State::Ready); };
    }
    //записать Р
    auto writeOutP(unsigned amp)
    {
        return [this, amp]() {
            const float k = 2.f * M_PI / static_cast<float>(N);
            for (size_t idx = 0; idx < N; ++idx) {
                OutP->raw[idx] = std::lround(amp * std::sin(k * idx) / 2.f + amp / 2.f);
            }
            this->OutP->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->OutP->getAddress(), Service::State::Ready); };
    }
    //записать напряжение
    auto writeOutN(unsigned amp)
    {
        return [this, amp]() {
            const float k = 2.f * M_PI / static_cast<float>(N);
            for (size_t idx = 0; idx < N; ++idx) {
                OutN->raw[idx] = std::lround(amp * std::sin(k * idx + M_PI) / 2 + amp / 2);
            }
            this->OutN->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->OutN->getAddress(), Service::State::Ready); };
    }
    //установить частоту
    auto setFreqOut()
    {
        return [this]() {
            this->out->freq = this->Frequency();
            this->out->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->out->getAddress(), Service::State::Ready); };
    }
    //установить усиление выхода
    auto setGainOut(coefOutService::gain gain)
    {
        return [this, gain]() {
            this->coefsOut->CurrentGain = gain;
            this->coefsOut->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsOut->getAddress(), Service::State::Ready); };
    }
    //установить усиление первого входного канала
    auto setGainIn1(coefInService::gain gain)
    {
        return [this, gain]() {
            this->coefsIn1->CurrentGain = gain;
            this->coefsIn1->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn1->getAddress(), Service::State::Ready); };
    }
    //установить усиление второго входного канала
    auto setGainIn2(coefInService::gain gain)
    {
        return [this, gain]() {
            this->coefsIn2->CurrentGain = gain;
            this->coefsIn2->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->coefsIn2->getAddress(), Service::State::Ready); };
    }
    //начать калибровку
    auto startCalibration()
    {
        return [this]() {
            this->control->command = controlService::startCalibration;
            this->control->transmit(1, Service::Type::TransmitStreaming);
            return std::make_pair(this->control->getAddress(), Service::State::Ready); };
    }
    //добавление процессов на колибровку выхода(минимум\максимум)
    void addProcessOut(processID proc, coefOutService::gain gain, unsigned min, unsigned max)
    {
        processes.add(proc | flagMin, writeOutN(min));//минимум напряжения
        processes.add(proc | flagMin, writeOutP(min));//минимум Р
        processes.add(proc | flagMin, setGainOut(gain));//усиление выхода
        processes.add(proc | flagMin, setFreqOut());//частота
        processes.add(proc | flagMin, startCalibration());//начать калибровку
        processes.add(proc, writeOutN(max));//максимум напряжения
        processes.add(proc, writeOutP(max));//максимум Р
        processes.add(proc, setGainOut(gain));//усиление выхода
        processes.add(proc, setFreqOut());//частота
        processes.add(proc, startCalibration());//начать калибровку
    }
    //добавление процессов на колибровку входа(1\2 канал, миниум\максимум)
    void addProcessIn(processID proc, coefInService::gain gainIn, coefOutService::gain gainOutMin, coefOutService::gain gainOutMax, unsigned min, unsigned max)
    {
        processes.add(proc | flagInCh1 | flagMin, writeOutN(min));//минимум напряжения
        processes.add(proc | flagInCh1 | flagMin, writeOutP(min));//минимум Р
        processes.add(proc | flagInCh1 | flagMin, setGainOut(gainOutMin));//усиление выхода
        processes.add(proc | flagInCh1 | flagMin, setGainIn1(gainIn));//усиление входа первого канала
        processes.add(proc | flagInCh1 | flagMin, setFreqOut());//частота
        processes.add(proc | flagInCh1 | flagMin, startCalibration());//начать калибровку
        processes.add(proc | flagInCh1, writeOutN(max));//максимум напряжения
        processes.add(proc | flagInCh1, writeOutP(max));//максимум Р
        processes.add(proc | flagInCh1, setGainOut(gainOutMax));//усиление выхода
        processes.add(proc | flagInCh1, setGainIn1(gainIn));//усиление входа первого канала
        processes.add(proc | flagInCh1, setFreqOut());//частота
        processes.add(proc | flagInCh1, startCalibration());//начать калибровку
        processes.add(proc | flagMin, writeOutN(min));//минимум напряжения
        processes.add(proc | flagMin, writeOutP(min));//минимум Р
        processes.add(proc | flagMin, setGainOut(gainOutMin));//усиление выхода
        processes.add(proc | flagMin, setGainIn2(gainIn));//усиление входа второго канала
        processes.add(proc | flagMin, setFreqOut());//частота
        processes.add(proc | flagMin, startCalibration());//начать калбировку
        processes.add(proc, writeOutN(max));//максимум напряжения
        processes.add(proc, writeOutP(max));//максимум Р
        processes.add(proc, setGainOut(gainOutMax));//усиление выхода
        processes.add(proc, setGainIn2(gainIn));//усиление входа второго канала
        processes.add(proc, setFreqOut());//частота
        processes.add(proc, startCalibration());//начать калибровку
    }


public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    //чтение
    void readyRead();
    //проверка ошибок порта
    void portError(QSerialPort::SerialPortError);
    //обновить порты
    void on_btnRefresh_clicked();
    //подключится
    void on_btnConnect_clicked();
    //отключиться
    void on_btnDisconnect_clicked();
    //старт калибровки
    void on_btnStart_clicked();
    //стоп
    void on_btnStop_clicked();
    //сохранить
    void on_btnSave_clicked();
    //загрузить
    void on_btnLoad_clicked();
    //зафиксировать значения при калибровке входа
    void on_btnFixIn_clicked();
    //выбор входного канала
    void on_btnCalIn_clicked();
    //зафиксировать значения
    void on_btnFixOut_clicked();
    //Калибровка выхода
    void on_btnCalOut_clicked();
    //изменения выходного усиления
    void on_lstOutGain_currentIndexChanged(int index);
    //изменение входного усидения
    void on_lstInGain_currentIndexChanged(int index);
    //редактирование теста
    void on_textLog_textChanged();
    //коэф. 1го канала
    void on_btnCoefIn1_clicked();
    //коэф. 2го канала
    void on_btnCoefIn2_clicked();
    //выходной коэф.
    void on_btnCoefOut_clicked();

private:
    float in1;//данные с 1 канала
    float in1_min;//минимум первого канала
    float in1_max;//максимум первого канала
    float in2;//данные со 2 канала
    float in2_min;//минимум 2 канала
    float in2_max;//максимум 2 канала
    float phase1;//фаза 1 канала
    float phase2;//фаза 2 канала

    Processes                             processes{ std::bind(&MainWindow::eventProcess, this, std::placeholders::_1) };//буфер процессов
    Dispatcher                            dispatcher;//диспетчер сервисов
    std::shared_ptr<Transport> transport = std::make_shared<Transport>(1, std::bind(&MainWindow::Write, this, _1,_2));//указатель на транспортный уровень
    std::shared_ptr<controlService>       control = makeService<controlService>(this, &MainWindow::notify, Services::Control);//контроллер команд
    std::shared_ptr<eventService>         message = makeService<eventService>(this, &MainWindow::notify, Services::Event);//контроллер евентов
    std::shared_ptr<qOutput>              out = makeService<qOutput>(this, &MainWindow::notify, Services::Output);//вывод настроек
    std::shared_ptr<parameterService>     parameter = makeService<parameterService>(this, &MainWindow::notify, Services::Mode);//параметры источника
    std::shared_ptr<measureService>       measure = makeService<measureService>(this, &MainWindow::notify, Services::Parameters);//буфер измеренных данных
    std::shared_ptr<qCoefOut>             coefsOut = makeService<qCoefOut>(this, &MainWindow::notify, Services::CoefsOut);//коэфиценты выхода
    std::shared_ptr<qCoefIn>              coefsIn1 = makeService<qCoefIn>(this, &MainWindow::notify, Services::CoefsIn1);//коэфиценты входа
    std::shared_ptr<qCoefIn>              coefsIn2 = makeService<qCoefIn>(this, &MainWindow::notify, Services::CoefsIn2);//коэфиценты входа
    std::shared_ptr<rawService<uint32_t>> OutP = makeService<rawService<uint32_t>>(this, &MainWindow::notify, Services::Reference1);//буфер чистых данных
    std::shared_ptr<rawService<uint32_t>> OutN = makeService<rawService<uint32_t>>(this, &MainWindow::notify, Services::Reference2);//буфер чистых данных

    CycleBuffer<float> buf{ AVG };//цыклический буфер

    QSerialPort* port;//порт

    Ui::mainwindow *ui;//пользовательский интерфейс
};
