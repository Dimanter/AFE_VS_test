﻿#pragma once

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
#include "output.hpp"

#include "protocol.hpp"

#include "Dispatcher.hpp"
#include "Process.hpp"
#include "controlService.hpp"
#include "eventService.hpp"
#include "measureService.hpp"
#include "stepService.hpp"

/**
 * @brief Work класс отображения результатов измерения датчика угал
 */
class Work : public QObject
{
	Q_OBJECT

		enum processID : Processes::processID_t
	{
		processMonitor,
		processMeasure,
		processBackward,
		processStart
	};
public:
	enum class Stage
	{
		Idle,
		Monitoring,
		Measurement
	};
	/**
	 * @brief Загрузка данных из сервиса
	 */
	auto load()
	{
		return [this]()
			{
				this->control->command = controlService::Load;
				this->control->transmit(1, Service::Type::TransmitConfirmed);
				return std::make_pair(this->control->getAddress(), Service::State::Ready);
			};
	}
	/**
	 * @brief Установить параметры вращения мотора
	 * @param dir Направление вращения
	 * @param ratio Дробный шаг, знаменатель
	 * @param period Период импульсов ШД
	 * @param count Количество шагов на измерительный цикл
	 */
	auto setStepper(stepDirection dir, stepRatio ratio, uint32_t period, uint32_t count)
	{
		return [this, dir, ratio, period, count]()
			{
				this->step->direction = dir;
				this->step->ratio = ratio;
				this->step->period = period;
				this->step->step = count;
				this->step->transmit(1, Service::Type::TransmitConfirmed);
				return std::make_pair(this->step->getAddress(), Service::State::Ready);
			};
	}
	/**
	 * @brief Запустить шаговый мотор
	 */
	auto startStepper()
	{
		return [this]()
			{
				this->control->command = controlService::startPosition;
				this->control->transmit(1, Service::Type::TransmitConfirmed);
				return std::make_pair(this->message->getAddress(), Service::State::Ready);
			};
	}

	auto SignalToSwitch()
	{
		return [this]()
			{
				this->control->command = controlService::Idle;
				this->control->transmit(1, Service::Type::TransmitConfirmed);
				return std::make_pair(this->message->getAddress(), Service::State::Ready);
			};
	}
	/**
	 * @brief Настроить вывод
	 */
	auto setOutput()
	{
		/*
		* Ввод частоты и фазы в микроконтроллер
		*/
		return [this]()
			{
				//Положительный
				this->out->outP = 3850.f;
				this->out->meanP = 1925.f;
				this->out->phaseP = 0.f;

				//Отрицательный
				this->out->outN = 3850.f;
				this->out->meanN = 1925.f;
				this->out->phaseN = M_PI;
				//Частота и ввод
				this->out->freq = 12000.f;
				this->out->transmit(1, Service::Type::TransmitConfirmed);
				return std::make_pair(this->out->getAddress(), Service::State::Ready);
			};
	}
	/// @brief Метод обратного вызова, уведомления о завершении процесса
	/// @param id идентификатор процесса
	void eventProcess(const Processes::processID_t& id)
	{

		switch (id) {
		case processStart:
			emit processComplete();
			Finished = true;
			break;
		case processMeasure:
			emit measureComplete();

			//board->Analyze();
			break;
		case processBackward:
			stage = Stage::Idle;
			break;
		}
	}

	/// @brief Метод обратного вызова, уведомления о состоянии сервиса
	/// @param service идентификатор сервиса
	/// @param st состояние сервиса
	void notify(const Service::serviceID_t& service, const Service::State& st)
	{
		processes.notify(service, st);

		if (st == Service::State::Ready)
		{
			switch (static_cast<Services>(service))
			{
			case Services::Event:
				switch (*message->event)
				{
				case eventService::pressButton:
					if (stage != Stage::Idle)
					{
						Stop();
					}
					else {
						Measure();
					}
					break;
				}
				break;
			case Services::Parameters:
				//board->setParameters(*measure->refAngle, *measure->Result);
				//board->update();
				break;
			}
		}
	}
	/**
	 * @brief Запись данных в порт
	 */
	uint8_t Write(std::unique_ptr<addressing_t[]> data, std::size_t size)
	{
		return port->write(reinterpret_cast<const char*>(data.get()), size);
	}

signals:
	void portStatus(bool);
	void readState();
	void measureComplete();
	void processComplete();

public:
	/// @brief Конструктор объекта управления процессом измерения
	/// @param device идентификатор проверяемого устройства
	/// @param width ширина области отображения результатов измерения
	/// @param height высота области отображения результатов измерения
	Work(const QString& device, qreal width, qreal height)
	{
		dispatcher.registerTransport(transport);

		dispatcher.registerService(control);
		dispatcher.registerService(message);
		dispatcher.registerService(out);
		dispatcher.registerService(step);
		dispatcher.registerService(measure);

		try
		{
			port = new QSerialPort(this);
			port->setBaudRate(7372800);
			port->setDataBits(QSerialPort::Data8);
			port->setParity(QSerialPort::NoParity);
			port->setStopBits(QSerialPort::OneStop);
			port->setFlowControl(QSerialPort::HardwareControl);

			connect(port, &QSerialPort::readyRead, this, &Work::readyRead);
			connect(port, &QSerialPort::errorOccurred, this, &::Work::portError);
			connect(this, &::Work::measureComplete, this, &::Work::Backward);
		}
		catch (...)
		{
			qDebug("Do not create serial port object");
		}

		//board = new BoardGraphicsItem(width, height);

		processes.add(processMonitor, load());
		processes.add(processMonitor, setOutput());
		processes.add(processMonitor, [this]() {
			this->control->command = controlService::startMonitor;
			this->control->transmit(1, Service::Type::TransmitConfirmed);
			return std::make_pair(this->control->getAddress(), Service::State::Ready);
			});

		processes.add(processMeasure, load());
		processes.add(processMeasure, setOutput());
		processes.add(processMeasure, setStepper(stepDirection::Backward, stepRatio::_1, 1200U, 2700U));
		processes.add(processMeasure, startStepper());
		processes.add(processMeasure, [this]() {
			this->control->command = controlService::startMeasuremnt;
			this->control->transmit(1, Service::Type::TransmitConfirmed);
			return std::make_pair(this->control->getAddress(), Service::State::Ready);
			});
		processes.add(processMeasure, setStepper(stepDirection::Forward, stepRatio::_1_32, 200U, 172800U));
		processes.add(processMeasure, startStepper());

		processes.add(processBackward, setStepper(stepDirection::Backward, stepRatio::_1, 1200U, 2700U));
		processes.add(processBackward, startStepper());
	}
	void portError(QSerialPort::SerialPortError error)
	{
		switch (error)
		{
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



	/*void setPos(qreal x, qreal y)
	{
		board->setPos(x, y);
	}*/

	bool Connect(QString portName)
	{
		if (!port->isOpen())
		{
			port->setPortName(portName);
			try {
				if (port->open(QIODevice::ReadWrite))
				{
					if (port->isOpen())
					{
						port->reset();
						dispatcher.resetService();
						qDebug() << (port->portName() + " >> Open!").toLocal8Bit();

					}
					else
					{
						port->close();
						qDebug() << port->errorString().toLocal8Bit();
						return false;
					}
				}
			}
			catch (...)
			{
				qDebug() << "Do not open serial port.";
				return false;
			}
		}
		return true;
	}

	void Disconnect()
	{
		if (port->isOpen())
		{
			port->close();
			qDebug() << (port->portName() + " >> Close!").toLocal8Bit();
			qDebug() << port->errorString().toLocal8Bit();
		}
	}

	bool isOpen()
	{
		return port->isOpen();
	}

	/*QGraphicsItem* getBoardItem()
	{
		return board;
	}

	QRectF getBoundingRect() const
	{
		return board->boundingRect();
	}*/

	void testCommand()
	{
		Finished = false;
		processes.clear(processStart);
		processes.add(processStart, load());
		processes.add(processStart, setOutput());
		processes.add(processStart, setStepper(stepDirection::Backward, stepRatio::_1, 1300, 600));
		processes.add(processStart, startStepper());
		processes.add(processStart, [this]() {
			this->control->command = controlService::startMeasuremnt;
			this->control->transmit(1, Service::Type::TransmitConfirmed);
			return std::make_pair(this->control->getAddress(), Service::State::Ready);
			});
		processes.add(processStart, setStepper(stepDirection::Forward, stepRatio::_1_2, 900, 125000));
		//processes.add(processStart, setStepper(stepDirection::Forward, stepRatio::_1, 1300, 49000));
		processes.add(processStart, startStepper());
		//processes.add(processStart, setStepper(stepDirection::Backward, stepRatio::_1, 1300, 22000));
		//processes.add(processStart, startStepper());
	}

	bool ProccesRunning()
	{
		return processes.getStatus(processStart);
	}

	bool ProccessStatus()
	{
		return Finished;
	}

	void processRun()
	{
		stage = Stage::Measurement;
		processes.run(processStart);
	}

	void processRunMonitor()
	{
		stage = Stage::Monitoring;
		processes.run(processStart);
	}
	/*@brief Метод подготовки мониторинга данных с выбранным вращением
	* @param Dir - направление вращения мотора
	* @param StepRate - длинна шага мотора
	* @param Period - период импульсов
	* @param Count - количество шагов
	*/
	void ReadyStartMonitor(int Dir, int StepRate, int Period, int Count)
	{
		stepDirection direction;
		if (Dir == 0)
			direction = stepDirection::Forward;
		else
			direction = stepDirection::Backward;
		processes.clear(processStart);
		stepRatio ratio;
		switch (StepRate)
		{
		case 1:
		{
			ratio = stepRatio::_1;
			break;
		}
		case 2:
		{
			ratio = stepRatio::_1_2;
			break;
		}
		case 4:
		{
			ratio = stepRatio::_1_4;
			break;
		}
		case 8:
		{
			ratio = stepRatio::_1_8;
			break;
		}
		case 16:
		{
			ratio = stepRatio::_1_16;
			break;
		}
		case 32:
		{
			ratio = stepRatio::_1_32;
			break;
		}
		case 64:
		{
			ratio = stepRatio::_1_64;
			break;
		}
		case 128:
		{
			ratio = stepRatio::_1_128;
			break;
		}
		case 256:
		{
			ratio = stepRatio::_1_256;
			break;
		}
		}

		processes.add(processStart, load());
		processes.add(processStart, setOutput());
		processes.add(processStart, [this]() {
			this->control->command = controlService::startMonitor;
			this->control->transmit(1, Service::Type::TransmitConfirmed);
			return std::make_pair(this->control->getAddress(), Service::State::Ready);
			});
		processes.add(processStart, setStepper(direction, ratio, Period, Count));
		processes.add(processStart, startStepper());
	}

	/*@brief Метод подготовки процессов для тестирования прибора
	* @param Dir - направление вращения мотора
	* @param StepRate - длинна шага мотора
	* @param Period - период импульсов
	* @param Count - количество шагов
	*/
	void ReadyStart(int Dir, int StepRate, int Period, int Count)
	{
		stepDirection direction;
		if (Dir == 0)
			direction = stepDirection::Forward;
		else
			direction = stepDirection::Backward;
		processes.clear(processStart);
		stepRatio ratio;
		switch (StepRate)
		{
		case 1:
		{
			ratio = stepRatio::_1;
			break;
		}
		case 2:
		{
			ratio = stepRatio::_1_2;
			break;
		}
		case 4:
		{
			ratio = stepRatio::_1_4;
			break;
		}
		case 8:
		{
			ratio = stepRatio::_1_8;
			break;
		}
		case 16:
		{
			ratio = stepRatio::_1_16;
			break;
		}
		case 32:
		{
			ratio = stepRatio::_1_32;
			break;
		}
		case 64:
		{
			ratio = stepRatio::_1_64;
			break;
		}
		case 128:
		{
			ratio = stepRatio::_1_128;
			break;
		}
		case 256:
		{
			ratio = stepRatio::_1_256;
			break;
		}
		}

		processes.add(processStart, load());
		processes.add(processStart, setOutput());
		processes.add(processStart, [this]() {
			this->control->command = controlService::startMeasuremnt;
			this->control->transmit(1, Service::Type::TransmitConfirmed);
			return std::make_pair(this->control->getAddress(), Service::State::Ready);
			});
		processes.add(processStart, setStepper(direction, ratio, Period, Count));
		processes.add(processStart, startStepper());
	}
	/*@brief Метод подготовки процессов для тестирования прибора с отступом в начале
	* @param Dir - направление вращения мотора
	* @param StepRate - длинна шага мотора
	* @param Period - период импульсов
	* @param Count - количество шагов
	* @param Offset - флаг отступа вначале
	*/
	void ReadyStart(int Dir, int StepRate, int Period, int Count, bool Offset)
	{
		stepDirection direction;
		if (Dir == 0)
			direction = stepDirection::Forward;
		else
			direction = stepDirection::Backward;
		processes.clear(processStart);
		stepRatio ratio;
		switch (StepRate)
		{
		case 1:
		{
			ratio = stepRatio::_1;
			break;
		}
		case 2:
		{
			ratio = stepRatio::_1_2;
			break;
		}
		case 4:
		{
			ratio = stepRatio::_1_4;
			break;
		}
		case 8:
		{
			ratio = stepRatio::_1_8;
			break;
		}
		case 16:
		{
			ratio = stepRatio::_1_16;
			break;
		}
		case 32:
		{
			ratio = stepRatio::_1_32;
			break;
		}
		case 64:
		{
			ratio = stepRatio::_1_64;
			break;
		}
		case 128:
		{
			ratio = stepRatio::_1_128;
			break;
		}
		case 256:
		{
			ratio = stepRatio::_1_256;
			break;
		}
		}
		processes.add(processStart, load());
		processes.add(processStart, setOutput());
		if (Offset)
		{
			processes.add(processStart, setStepper(stepDirection::Backward, stepRatio::_1, 1300, 850));
			processes.add(processStart, startStepper());
		}


		processes.add(processStart, [this]() {
			this->control->command = controlService::startMeasuremnt;
			this->control->transmit(1, Service::Type::TransmitConfirmed);
			return std::make_pair(this->control->getAddress(), Service::State::Ready);
			});
		processes.add(processStart, setStepper(direction, ratio, Period, Count));
		processes.add(processStart, startStepper());
	}
	virtual ~Work() {}


public slots:
	void Monitor(void)
	{
		if (port->isOpen())
		{
			if (stage == Stage::Idle)
			{
				processes.run(processMonitor);
				stage = Stage::Monitoring;
			}
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText("Port is not open.");
			msgBox.exec();
		}
	}

	void Measure(void)
	{
		if (port->isOpen())
		{
			if (stage == Stage::Idle)// && board->showInfo()) 
			{
				//board->Clear();
				processes.run(processMeasure);
				stage = Stage::Measurement;
			}
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText("Port is not open.");
			msgBox.exec();
		}
	}

	void Stop(void)
	{
		if (port->isOpen())
		{
			Finished = false;
			control->command = controlService::Stop;
			control->transmit(1, Service::Type::TransmitStreaming);
			stage = Stage::Idle;
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText("Port is not open.");
			msgBox.exec();
		}
	}

private slots:
	void Backward()
	{
		if (port->isOpen())
		{
			processes.run(processBackward);
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText("Port is not open.");
			msgBox.exec();
		}
	}

	void readyRead()
	{
		auto data = port->readAll();
		transport->receive(reinterpret_cast<unsigned char*>(data.data()), data.size());
	}

public:
	bool Finished = false;
	Stage              stage{ Stage::Idle };
	QSerialPort* port;
	//BoardGraphicsItem* board;
	QWidget ResulWin;
	Processes                       processes{ std::bind(&Work::eventProcess, this, std::placeholders::_1) };
	Dispatcher                      dispatcher;
	std::shared_ptr<Transport>      transport = std::make_shared<Transport>(1, std::bind(&Work::Write, this, _1, _2));
	std::shared_ptr<controlService> control = makeService<controlService>(this, &Work::notify, Services::Control);
	std::shared_ptr<eventService>   message = makeService<eventService>(this, &Work::notify, Services::Event);
	std::shared_ptr<stepService>    step = makeService<stepService>(this, &Work::notify, Services::Step);
	std::shared_ptr<qOutput>        out = makeService<qOutput>(this, &Work::notify, Services::Output);
	std::shared_ptr<measureService> measure = makeService<measureService>(this, &Work::notify, Services::Parameters);
	std::shared_ptr<measureService> stepWin = makeService<measureService>(this, &Work::notify, Services::Parameters);
};
