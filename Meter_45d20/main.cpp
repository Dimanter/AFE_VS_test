#include "Meter_45d20.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
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
    Meter w;
    w.show();
    return 0; //a.exec();
}
