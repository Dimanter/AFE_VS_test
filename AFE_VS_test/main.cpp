#pragma once
#include "MainWindow.hpp"
#include "Monitoring.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>

/*
* 1 Загрузка данных (частоты и фазы) Work.hpp
* 2 Запуск шагового мотора Work.hpp
* 3 Измерения угла/ перевод в секунды минуты Board.cpp
* 4 Измерение фазы, наряжения, силы тока Board.cpp
* 5 Анализ измерениЙ? Board.cpp
*/
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption portOption(QStringList() << "d" << "device", "Device type: 45Д20-2; СКТ-232Б.", "Device", "45Д20-2");
    parser.addOption(portOption);
    parser.process(a);

    //включить для программы монитора
    /*Monitoring w{};
    QCoreApplication::setApplicationName("Монитор");
    w.show();
    return a.exec();*/

    //включить для программы измерений
    StepWindow s{ "45Д20-2" }; // 45Д20-2; СКТ-232Б
    QCoreApplication::setApplicationName("Метер");
    s.show();
    return 0;
}