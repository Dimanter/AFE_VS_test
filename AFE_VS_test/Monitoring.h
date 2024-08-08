#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <map>

#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>

#include "Work.hpp"
#include "ui_Monitoring.h"

/*@brief Класс для хранения данных
*/
class Data
{
public:
	float angleGrad;// Угол в градусах с плавающей запятой
	int angle;// Угол в градусах целочисленный
	int min;// Минуты угла
	int sec;// Секунды угла
	float V1;// Напряжение на первом канале
	float V2;// Напряжение на втором канале
	float Phase1;// Фаза первого канала
	float Phase2;// фаза второго канала
	float I;// Сила тока
	float IPhase;// Фаза тока
};

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
	//@brief Метод считывающй данные с контроллера
	//@return Структура данных
	Data Read();
	/*@brief Метод усреднения данных по минутам угла
	* @param cont - контенер данных измеренных прибором
	* @returns Изменённый контенер данных
	*/
	vector<Data> AverageData(vector<Data> cont);
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
	vector<Data> data;//контенер измеренных данных
	QTimer* timer = new QTimer(this);// Таймер для вывода данных на интерфейс
	QTimer* timerAverage = new QTimer(this);// Таймер для усреднения данных
	bool connection;// Статус подключения к порту
};
