#include "MainWindow.hpp"
#include "StepWindow.hpp"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>

/*
* 1 Загрузка данных (частоты и фазы) Work.hpp
* 2 Запуск шагового мотора Work.hpp
* 3 Измерения угла/ перевод в секунды минуты Board.cpp
* 4 Измерение фазы, наряжения, силы тока Board.cpp
* 5 Анализ измерении? Board.cpp
*/
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setApplicationName("Measurement of the parameters of the angular position sensors.");

    QCommandLineParser parser;
    parser.setApplicationDescription("Measurement of the parameters of the angular position sensors");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption portOption(QStringList() << "d" << "device", "Device type: 45Д20-2; СКТ-232Б.", "Device", "45Д20-2");
    parser.addOption(portOption);
    parser.process(a);

    //MainWindow w{ parser.value(portOption) };
    StepWindow w{ "СКТ-232Б"};
    
    

    //w.setWindowTitle(QCoreApplication::applicationName());
    w.show();

    return 0;// a.exec();
}

