#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Monitor");
    QCoreApplication::setApplicationVersion("1.0");

    MainWindow w;
    w.setWindowTitle(QCoreApplication::applicationName());
    w.showMaximized();
    return a.exec();
}
