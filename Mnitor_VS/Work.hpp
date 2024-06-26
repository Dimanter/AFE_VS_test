#pragma once

#include <QObject>
#include <QtSerialPort/QSerialPort>

#include "board.hpp"
#include "../AFE_VS_test/output.hpp"

#include "../AFE_VS_test/protocol.hpp"

#include "../AFE_VS_test/Dispatcher.hpp"
#include "../AFE_VS_test/Process.hpp"
#include "../AFE_VS_test/controlService.hpp"
#include "../AFE_VS_test/eventService.hpp"
#include "../AFE_VS_test/measureService.hpp"

/**
 * @brief Work класс отображения результатов измерения датчика угал
 */
class Work : public QObject
{
    Q_OBJECT

        enum class Stage
    {
        Idle,
        PrepareMonitoring,
        Monitoring,
        Measurement
    };

    enum processID : Processes::processID_t
    {
        processMonitor,
        processMeasure,
        processGetOutSetting
    };

    void eventProcess(Processes::processID_t id)
    {
        switch (id) {
        case processGetOutSetting:
            showSetting();
            break;
        default:
            break;
        }
    }

    void notify(const Service::serviceID_t& service, const Service::State& st)
    {
        processes.notify(service, st);
        switch (static_cast<Services>(service)) {
        case Services::Event:
            switch (*message->event) {
            case eventService::pressButton:
                if (stage == Stage::Idle) {
                    processes.run(processMeasure);
                }
                else {
                    Stop();
                }
                break;
            }
            break;
        case Services::Parameters:
            board->setParameters(*measure->refAngle, *measure->Result);
            board->update();
            break;
        }
    }

    uint8_t Write(std::unique_ptr<addressing_t[]> data, std::size_t size)
    {
        return port->write(reinterpret_cast<const char*>(data.get()), size) != -1 ? 0U : 1U;
    }

signals:
    void portStatus(bool);

public:
    /**
     * @brief Display конструктор класса отображения значения угла
     * @param width ширина области отображения
     * @param height высота области отображения
     * @param portName имя порта получения результатов измерения
     */
    Work(qreal width, qreal height)
    {
        dispatcher.registerTransport(transport);

        dispatcher.registerService(control);
        dispatcher.registerService(message);
        dispatcher.registerService(out);
        dispatcher.registerService(measure);

        processes.add(processMonitor, [this]() {
            this->control->command = controlService::Load;
            this->control->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->control->getAddress(), Service::State::Ready);
            });
        processes.add(processMonitor, [this]() {
            this->control->command = controlService::startMonitor;
            this->control->transmit(1, Service::Type::TransmitStreaming);
            return std::make_pair(this->control->getAddress(), Service::State::Ready);
            });

        processes.add(processMeasure, [this]() {
            this->control->command = controlService::Load;
            this->control->transmit(1, Service::Type::TransmitConfirmed);
            return std::make_pair(this->control->getAddress(), Service::State::Ready);
            });
        processes.add(processMeasure, [this]() {
            this->control->command = controlService::startMeasuremnt;
            this->control->transmit(1, Service::Type::TransmitStreaming);
            return std::make_pair(this->control->getAddress(), Service::State::Ready);
            });

        processes.add(processGetOutSetting, [this]() {
            out->transmit(1, Service::Type::ReceiveStreaming);
            return std::make_pair(this->out->getAddress(), Service::State::Ready);
            });

        try {
            port = new QSerialPort(this);
            port->setBaudRate(7372800);
            port->setDataBits(QSerialPort::Data8);
            port->setParity(QSerialPort::NoParity);
            port->setStopBits(QSerialPort::OneStop);
            port->setFlowControl(QSerialPort::HardwareControl);

            connect(port, &QSerialPort::readyRead, this, &Work::readyRead);
            connect(port, &QSerialPort::errorOccurred, this, &::Work::portError);

            port->moveToThread(&thread);
            thread.start();
        }
        catch (...) {
            qDebug("Do not create serial port object");
        }

        board = new BoardGraphicsItem(width, height);
    }

    void portError(QSerialPort::SerialPortError error)
    {
        switch (error) {
        case QSerialPort::NoError:
            port->close();
            emit portStatus(true);
            break;
        case QSerialPort::ResourceError:
            port->close();
            emit portStatus(false);
            break;
        default:
            break;
        }
    }

    void setPos(qreal x, qreal y)
    {
        board->setPos(x, y);
    }

    void Connect(QString portName)
    {
        if (!port->isOpen()) {
            port->setPortName(portName);
            try {
                if (port->open(QIODevice::ReadWrite)) {
                    if (port->isOpen()) {
                        dispatcher.resetService();
                        qDebug() << (port->portName() + " >> Open!").toLocal8Bit();
                    }
                    else {
                        port->close();
                        qDebug() << port->errorString().toLocal8Bit();
                    }
                }
            }
            catch (...) {
                qDebug("Do not open serial port.");
            }
        }
    }

    void Disconnect()
    {
        if (port->isOpen()) {
            port->close();
            qDebug() << (port->portName() + " >> Close!").toLocal8Bit();
        }
    }

    bool isOpen()
    {
        return port->isOpen();
    }

    QGraphicsItem* getBoardItem()
    {
        return board;
    }

    QRectF getBoundingRect() const
    {
        return board->boundingRect();
    }

    ~Work()
    {
        Disconnect();
    }

public slots:
    void Monitor(void)
    {
        if (port->isOpen()) {
            processes.run(processMonitor);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Port is not open.");
            msgBox.exec();
        }
    }

    void Stop(void)
    {
        if (port->isOpen()) {
            control->command = controlService::Stop;
            control->transmit(1, Service::Type::TransmitStreaming);
            stage = Stage::Idle;
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Port is not open.");
            msgBox.exec();
        }
    }

    void Reset(void)
    {
        if (port->isOpen()) {
            control->command = controlService::resetEncoder;
            control->transmit(1, Service::Type::TransmitStreaming);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Port is not open.");
            msgBox.exec();
        }
    }

    void Settings(void)
    {
        if (port->isOpen()) {
            processes.run(processGetOutSetting);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Port is not open.");
            msgBox.exec();
        }
    }

    void Save(void)
    {
        board->Save();
    }

    void Clear(void)
    {
        board->Clear();
    }

private slots:
    void readyRead()
    {
        auto data = port->readAll();
        transport->receive(reinterpret_cast<unsigned char*>(data.data()), data.size());
    }

    void showSetting(void)
    {
        if (out->exec() == QDialog::Accepted) {
            out->transmit(1, Service::Type::TransmitStreaming);
        }
    }

private:
    Stage              stage{ Stage::Idle };
    QThread            thread;
    QSerialPort* port;
    BoardGraphicsItem* board;

    Processes                       processes{ std::bind(&Work::eventProcess, this, std::placeholders::_1) };
    Dispatcher                      dispatcher;
    std::shared_ptr<Transport>      transport = std::make_shared<Transport>(1, std::bind(&Work::Write, this, _1, _2));
    std::shared_ptr<controlService> control = makeService<controlService>(this, &Work::notify, Services::Control);
    std::shared_ptr<eventService>   message = makeService<eventService>(this, &Work::notify, Services::Event);
    std::shared_ptr<qOutput>        out = makeService<qOutput>(this, &Work::notify, Services::Output);
    std::shared_ptr<measureService> measure = makeService<measureService>(this, &Work::notify, Services::Parameters);
};
