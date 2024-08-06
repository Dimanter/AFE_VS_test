
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);//создаём приложение
    QCoreApplication::setApplicationName("Calibrate");//устанавливаем имя приложения
    QCoreApplication::setApplicationVersion("1.01");//устанавливаем версию приложения

    MainWindow w;//создаём главное окно
    w.setWindowTitle(QCoreApplication::applicationName());//устанавливаем заголовок в виде имени приложения
    w.setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);//устанавливаем флаги приложения (окно, без кнопки скрытия, с кнопкой помощь)
    w.show();//показываем главное окно
    return a.exec();
}
