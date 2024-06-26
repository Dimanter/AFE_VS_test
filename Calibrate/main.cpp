
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Calibrate");
    QCoreApplication::setApplicationVersion("1.0");

    MainWindow w;
    w.setWindowTitle(QCoreApplication::applicationName());
    w.setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
    w.show();
    return a.exec();
}
