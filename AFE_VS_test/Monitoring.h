#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <map>

#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>

#include "StepWindow.hpp"
#include "ui_Monitoring.h"

class Monitoring : public QMainWindow
{
	Q_OBJECT

public:
	Monitoring(QWidget *parent = nullptr);
	~Monitoring();
	//@brief Метод подключение кнопок к функциям
	void Create();
	//@brief Метод обновления доступных com-портов
	void Refresh();
	//@brief Метод поключения к com-порту
	void Connect();
	//@brief Метод отключения от com-порта
	void Disconnect();
	//@brief Метод останавливающий все процессы на контроллере
	void Stop();
	//@brief Метод обновления данных на пользовательском интерфейсе
	void StepThreading();
	//@brief Метод запускающий процесс мониторинга
	void Monitor();
	//@brief Метод проверяющий статус подключения
	//@param connected текущий статус
	void StatusConnect(bool connected);
	//@brief Метод конвертирующий угол с плавающий точкой в угол с минутами и секундами
	//@param _angle угол для конвертирования
	void ConvertAngle(float _angle);
	//@brief Метод изменяющий частоту и напряжение на контроллере
	void ChangeSettings();
	//void Start();
	void AboutProgramm();
	//@brief Метод считывающй данные с контроллера
	//@return Структура данных
	Data Read();
	/*@brief Метод поиска минимального напряжения
	* @param cont - контенер данных измеренных прибором
	* @returns Индекс минимального угла
	*/
	int FindMinV(vector<Data> cont);
	/*@brief Метод записи данных в файл
	* @param file - имя фалйа для записи
	* @param cont - контенер данных измеренных прибором
	*/
	void Serialization(string file, vector<Data> cont);
	/*@brief Метод усреднения данных по минутам угла
	* @param cont - контенер данных измеренных прибором
	* @returns Изменённый контенер данных
	*/
	Data AverageData(vector<Data> &dataVector);
	/*@brief Метод удаления ошибочных данных
	* @param cont - контенер данных измеренных прибором
	* @returns Изменённый контенер данных
	*/
	vector<Data> EraseErrors(vector<Data> cont);
	/*@brief Метод проверяющий содержится ли заданный угол в контенере данных
	* @param cont - контенер данных измеренных прибором
	* @param _angle - угол в градусах целочисленный
	* @param _min - минуты угла
	* @returns true, false
	*/
	bool Contains(vector<Data> cont, int _angle, int _min);
	/*@brief Метод конвертации напряжения в массив данных
	* @param cont - контенер данных измеренных прибором
	* @returns Массив данных напряжения
	*/
	float* VectorInMassV(vector<Data> cont, bool V1);
	/*@brief Метод конвертации угла в массив данных
	* @param cont - контенер данных измеренных прибором
	* @param mins - флаг конвертации минут или градусов угла(true - минуты,false - угол)
	* @returns Массив данных угла
	*/
	int* VectorInMass(vector<Data> cont, bool mins);

private slots:
	/*
	*@brief Обновление данных на интерфейсе
	*/
	void Update();
	/*
	*@brief Усреднение данных
	*/
	void Average();

private:
	Ui::MonitoringClass ui;
	Work* work;
	int angle = 0;// Угол в градусах целочисленный
	int min = 0;// Минуты угла
	int sec = 0;// Секунды Угла
	float secDec = 0;
	vector<Data> data;//контенер измеренных данных
	QTimer* timer = new QTimer(this);// Таймер для вывода данных на интерфейс
	QTimer* timerAverage = new QTimer(this);// Таймер для усреднения данных
	bool connection;// Статус подключения к порту
};
