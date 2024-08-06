#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <map>

#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>

#include "Work.hpp"
#include "ui_Monitoring.h"

class Monitoring : public QMainWindow
{
	Q_OBJECT

public:
	Monitoring(QWidget *parent = nullptr);
	~Monitoring();
	void Create();
	void Refresh();
	void Connect();
	void Disconnect();
	void Stop();
	void StepThreading();
	void Monitor();
	void StatusConnect(bool connected);
	void ConvertAngle(float _angle);
	void ChangeSettings();

private slots:
	/*
	*@brief Обновление данных на интерфейсе
	*/
	void Update();

private:
	Ui::MonitoringClass ui;
	Work* work;
	int angle = 0;// Угол в градусах целочисленный
	int min = 0;// Минуты угла
	int sec = 0;// Секунды Угла
	QTimer* timer = new QTimer(this);// Таймер для вывода данных на интерфейс
	bool connection;
};
