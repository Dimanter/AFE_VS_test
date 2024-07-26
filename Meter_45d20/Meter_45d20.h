#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Meter_45d20.h"
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

#include <QtWidgets>

class Meter : public QMainWindow
{
    Q_OBJECT

public:
    Meter(QWidget* parent = nullptr);
    void CreateWindow();
    void RefreshComboBox();
    void Start();
    void Connect();
    void Disconnect();
    void Stop();
    void StartMonitor();
    ~Meter();

private:
    QWidget* window = new QWidget();
    QComboBox* comboPort1;
    QPushButton* btnConnect;// Кнопка подклчения
    QPushButton* btnDisconnect;// Кнопка отключения
    QPushButton* btnStart;// Кнопка старт
    QPushButton* btnRefreshPort;// Кнопка обновления com-портов
    QPushButton* btnStop;// Кнопка остановки процессов
};
