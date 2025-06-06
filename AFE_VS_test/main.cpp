﻿#pragma once
#include "StartWindow.hpp"

/*
* 1 Загрузка данных (частоты и фазы) Work.hpp
* 2 Запуск шагового мотора Work.hpp
* 3 Измерения угла/ перевод в секунды минуты Stepwindow.cpp
* 4 Измерение фазы, наряжения, силы тока StepWindow.cpp
* 5 Анализ измерений Stepwindow.cpp
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
    QCommandLineOption portOption(QStringList() << "d" << "device", "Device type: 45Д20-2; СКТ-232Б; СКТ-265Д.", "Device", "45Д20-2");
    parser.addOption(portOption);
    parser.process(a);


    //включить для программы монитора
    /*Monitoring w{};
    QCoreApplication::setApplicationName("Монитор");
    w.show();
    return a.exec();*/

    StartWindow main{};
    main.show();
    return 0;
}
//TODO: <17 a He 20 :<