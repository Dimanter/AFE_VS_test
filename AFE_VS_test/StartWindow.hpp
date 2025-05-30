#pragma once
#include "Monitoring.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>

class StartWindow : public QMainWindow
{
public:
	StartWindow(QWidget* parent = nullptr);
	int BtnSkt232();
	int Btn45D20();
	int BtnSkt265();
	void CreateWindow();
	~StartWindow();

	QPushButton* btnSkt232;// Кнопка 
	QPushButton* btnSkt265;// Кнопка 
	QPushButton* btn45D20;// Кнопка 

	QWidget* window = new QWidget();// Главное окно
};

