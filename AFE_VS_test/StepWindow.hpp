﻿#pragma once
#include "ui_AFE_VS_test.h"
#include <QGraphicsView>
#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QMdiArea>
#include <QTimer>
#include <QTime>
#include <map>

#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>

#include "Work.hpp"

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

/*@brief Класс управления шаговым двигателем и обработки данных
*/
class StepWindow : public QMainWindow
{
	Q_OBJECT
		Ui::StepWindow ui;
public:
	/*@brief Конструктор
	* @param Device - онформация о приборе
	* @param parent - указатель на родителя
	*/
	StepWindow(const QString& device, QWidget* parent = nullptr);
	/*@brief Метод проверяющий содержится ли заданный угол в контенере данных
	* @param cont - контенер данных измеренных прибором
	* @param _angle - угол в градусах с плавающей запятой
	* @returns true, false
	*/
	bool Contains(vector<Data> cont, float _agnle);
	/*@brief Метод проверяющий содержится ли заданный угол в контенере данных
	* @param cont - контенер данных измеренных прибором
	* @param _angle - угол в градусах целочисленный
	* @param _min - минуты угла
	* @returns true, false
	*/
	bool Contains(vector<Data> cont, int _agnle, int _min);
	/*@brief Метод проверяющий содержится ли заданный угол в контенере данных
	* @param cont - контенер данных измеренных прибором
	* @param _angle - угол в градусах целочисленный
	* @param _min - минуты угла
	* @param _sec - секунды угла
	* @returns true, false
	*/
	bool Contains(vector<Data> cont, int _angle, int _min, int _sec);
	/*@brief Метод проверяющий входит ли данный угол в диапазон поиска минимумов
	* @param _angle - угол в градусах целочисленный
	* @param _min - минуты угла
	* @returns true, false
	*/
	bool CheckDiap(int _angle, int _min);
	/*@brief Метод проверяющий все ли данные больше нуля
	* @param cont - контенер данных измеренных прибором
	* @returns true, false
	*/
	bool Minus(vector<int> cont);
	/*@brief Метод проверяющий все ли данные меньше нуля
	* @param cont - контенер данных измеренных прибором
	* @returns true, false
	*/
	bool Plus(vector<int> cont);
	/*@brief Метод проверки статуса подключения
	*/
	void StatusConnect(bool connected);
	/*@brief Метод вывода данных на интерфейс
	*/
	void StepThreading();
	/*@brief Отображение текущих параметров заданных для шагового мотора
	*/
	void Aply();
	/*@brief Отображение текущих параметров заданных для шагового мотора
	* @param _dir - направление движения (0 - вперёдт, 1 - назад)
	* @param _period - период импульсов
	* @param _step  - длинна шага (1, 1/2, 1/4...)
	* @param _count - количество шагов
	*/
	void AplyOpt(int _dir, int _period, int _step, int _count);
	/*@brief Метод создания пользовательского интерфейса
	*/
	void CreateWindow();
	/*@brief Метод покдлючения к com-порту
	*/
	void Connect();
	/*@brief Метод отключения от com-порта
	*/
	void Disconnect();
	/*@brief Метод остановки процессов измерения
	*/
	void Stop();
	/*@brief Метод запускающий шаговый двигатель с установленными параметрами
	*/
	void Start();
	/*@brief Метод конвертации угла с плавающей запятов в угол с минутами и секундами
	* @param _angle - угол с плавающей запятой
	*/
	void ConvertAngle(float angle);
	/*@brief Метод обработки данных после завершения измерений
	*/
	void TestComplete();
	/*@brief Метод старта мониторинга данных
	*/
	void StartMonitor();
	/*@brief Метод обновления доступных com-портов
	*/
	void RefreshComBox();
	/*@brief Метод тестирования прибора
	*/
	void testCommand();
	/*@brief Метод запускающий программу конвертер
	*/
	void StartUpConverter();
	/*@brief Метод записи данных в файл
	* @param file - имя фалйа для записи
	* @param cont - контенер данных измеренных прибором
	*/
	void Serialization(string file, vector<Data> cont);
	/*@brief Метод вызывающий диалоговое окно для записи названия изделия
	*/
	bool Dialog();
	/*@brief Метод обмена данными мужду двумя параметрами
	* @param val1 - первый параметр
	* @param val2 - второй параметр
	* @returns
	*/
	void SwapData(Data& val1, Data& val2);
	/*@brief Метод поиска максимального напряжения
	* @param cont - контенер данных измеренных прибором
	* @returns Индекс максимального угла
	*/
	int FindMaxV(vector<Data> cont);
	/*@brief Метод поиска минимального напряжения
	* @param cont - контенер данных измеренных прибором
	* @returns Индекс минимального угла
	*/
	int FindMinV(vector<Data> cont);
	/*@brief Метод поиска минимального напряжения
	* @param cont - контенер данных измеренных прибором
	* @param minAngle - угол от которого искать минимум
	* @param maxAngle - угол до которого искать минимум
	* @returns Индекс минимального угла
	*/
	int FindMinV(vector<Data> cont, int minAngle, int maxAngle);
	/*@brief Метод нахождения минимального угла
	* @param cont - контенер данных измеренных прибором
	* @returns Индекс минимального угла
	*/
	int FindMinAngle(vector<Data> cont);
	/*@brief Метод нахождения минимального смещения угла
	* @param Angle - массив углов
	* @returns Минимальный угол
	*/
	int MinOffset(vector<int> Angle);
	/*@brief Метод нахождения максимального смещения угла
	* @param Angle - массив угло
	* @returns Максимальный угол
	*/
	int MaxOffset(vector<int> Angle);
	/*@brief Метод конвертации угла в массив данных
	* @param cont - контенер данных измеренных прибором
	* @param mins - флаг конвертации минут или градусов угла(true - минуты,false - угол)
	* @returns Массив данных угла
	*/
	int* VectorInMass(vector<Data> cont, bool mins);
	/*@brief Метод вычисления среднего напряжения
	* @param cont - контенер данных измеренных прибором
	* @returns Среднее напряжение
	*/
	float AverageU(vector <Data> cont);
	/*@brief Метод конвертации напряжения в массив данных
	* @param cont - контенер данных измеренных прибором
	* @returns Массив данных напряжения
	*/
	float* VectorInMassV(vector<Data> cont);
	/*@brief Метод усреднения данных по минутам угла
	* @param cont - контенер данных измеренных прибором
	* @returns Изменённый контенер данных
	*/
	vector<Data> Average(vector<Data> cont);
	/*@brief Метод удаления ошибочных данных
	* @param cont - контенер данных измеренных прибором
	* @returns Изменённый контенер данных
	*/
	vector<Data> EraseErrors(vector<Data> cont);
	/*@brief Метод удаления одного градуса
	* @param cont - контенер данных измеренных прибором
	* @returns Изменённый контенер данных
	*/
	vector<Data> Erase(vector<Data> cont);
	/*@brief Метод удаления одного элемента
	* @param cont - контенер данных измеренных прибором
	* @param index - Индекс удаляемого элемента
	* @returns Изменённый контенер данных
	*/
	vector<Data> Erase(vector<Data> cont, int index);
	/*@brief Метод удаления диапазона элементов
	* @param cont - контенер данных измеренных прибором
	* @param left - Левая граница диапазона
	* @param right - Правая граница диапазона
	* @returns Изменённый контенер данных
	*/
	vector<Data> Erase(vector<Data> cont, int left, int right);
	/*@brief Метод сдвига угла и минут относительно минимума напряжения
	* @param cont - контенер данных измеренных прибором
	* @param minInd - индекс элемента с минимальным напряжением
	* @returns Изменённый контенер данных
	*/
	vector<Data> AngleOffset(vector<Data> cont, int minInd);
	/*@brief Метод считывания данных с файла
	* @param file - названия файла
	* @returns Контенер данных
	*/
	vector<Data> ReadFromFile(string file);
	/*@brief Метод запись данных в структуру
	* @returns Структура данных
	*/
	Data Read();
	/*@brief Деструктор
	*/
	~StepWindow();
private:



	QTimer* timer = new QTimer(this);// Таймер для вывода данных на интерфейс
	QTimer* testTimer = new QTimer(this);// Таймер для тестирования прибора

	Data temp;// Буффер для хранения данных

	vector<Data> data;// Контенер считанных данных
	vector<vector<Data>> allData;// Контенер данных разделённых на схемы комутации

	QStandardItemModel* model = new QStandardItemModel(15, 22);//Таблица расчёта данных
	QComboBox* comboPort1;// Раскрывающшийся список с доступными com-портами
	QComboBox* DirBox;//Направление движения
	QComboBox* Steps;//Длинна шага
	QLabel* label;// Заголовок
	QLabel* StatusCon;//Статус подключения
	QPlainTextEdit* plainTextEdit = new QPlainTextEdit();//Объект главного окна - текст вводных данных

	QColor red = QColor(255, 51, 51);//красный цвет в ргб
	QColor green = QColor(51, 153, 51);//зелёный цвет в ргб
	QColor yellow = QColor(218, 165, 32);//жёлтый цвет в ргб

	QPushButton* btnConnect;// Кнопка подклчения
	QPushButton* btnDisconnect;// Кнопка отключения
	QPushButton* btnStart;// Кнопка старт
	QPushButton* btnTest;// Кнопка теста
	QPushButton* btnRefreshPort;// Кнопка обновления com-портов
	QPushButton* btnStop;// Кнопка остановки процессов

	QTextEdit* textDir;// Текствоое окно для записи направления движения мотора
	QTextEdit* textPeriod;// Текстовое окно для записи периода импульсов
	QTextEdit* textCount;// Текствое окно для записи количества шагов
	QTextEdit* I = new QTextEdit("");// Текствое окно для вывода силы тока
	QTextEdit* PhaseV2 = new QTextEdit("");// Текстовое окно для вывода фазы второго канала
	QTextEdit* PhaseV1 = new QTextEdit("");// Текстовое окно для вывода фазы первого канала
	QTextEdit* RmsV2 = new QTextEdit("");// Текстовое окно для вывода напряжения воторого канала
	QTextEdit* RmsV1 = new QTextEdit("");// Текствовое окно для вывода напряжения первого канала
	QTextEdit* textAngle = new QTextEdit("");// Текствовое окно для вывода угла
	QTextEdit* IPhase = new QTextEdit("");// Текстовое окно для вывода фазы тока

	QString NumberDevice = "";//номер изделия
	int loop = 1;// Текущий круг измерений
	int fileNum = 0;//Кол-во файлов для конвертации
	int angle = 0;// Угол в градусах целочисленный
	int min = 0;// Минуты угла
	int sec = 0;// Секунды Угла
	int Dir;// Направление вращения мотора
	int StepRatio;// Длинна шага
	int Period;// Период импульсов
	int Count;// Количество шагов
	int Diap[5][2] = { {0,20},//Диапазоны поиска минимумов
					{45,65},
					{90,110},
					{180,200},
					{270,290} };
	int Ind[13] = { 0,0,2,2,3,3,4,4,1,1,0,2,1 };//Индексы заполнения градусов в таблицу
	map <string, unsigned> Punkts;//Пункты схем комутации
	vector<int> MinV;//Индексы минимумов напряжения

	bool connection = false;// Флаг полкючения порта
	bool LoopFlag = false;// Флаг обновления круга

	Work* work;// Экземпляр класса Work
	QWidget* window = new QWidget();// Главное окно

private slots:
	/*
	*@brief Обновление данных на интерфейсе
	*/
	void Update();
	/*
	*@brief Тест прибора
	*/
	void TestUp();
};