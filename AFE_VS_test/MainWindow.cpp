#include <QtWidgets>

#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>
#include <windows.h>
#include <locale.h>

#include "MainWindow.hpp"
#include "Board.hpp"
//@brief Конструктор с основыным графическим окном и кнопками
MainWindow::MainWindow(const QString& device, QWidget* parent) : QMainWindow(parent)
{
    
    scene = new QGraphicsScene();
    view = new QGraphicsView(scene);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    setCentralWidget(view);

    work = new Work(device, 2400, 1400);
    connect(work, &Work::portStatus, this, &MainWindow::statusConnect);

    ddlPort = new QComboBox;
    refreshPort();

    QToolBar* ptb = new QToolBar("Control ToolBar");

    ptb->addWidget(ddlPort);

    QAction* act = new QAction("Refresh");
    act->setText(("Refresh"));
    act->setShortcut(QKeySequence("CTRL+R"));
    act->setToolTip("Update the list of available serial ports\nCTRL+R");
    connect(act, SIGNAL(triggered()), this, SLOT(refreshPort()));
    ptb->addAction(act);

    actConnect = new QAction("Connect");
    actConnect->setText("Connect");
    actConnect->setShortcut(QKeySequence("CTRL+C"));
    actConnect->setToolTip("Connect device\nCTRL+C");
    connect(actConnect, SIGNAL(triggered()), this, SLOT(Connect()));
    ptb->addAction(actConnect);

    act = new QAction("Disconnect");
    act->setText("Disconnect");
    act->setShortcut(QKeySequence("CTRL+D"));
    act->setToolTip("Disconnect device\nCTRL+D");
    connect(act, SIGNAL(triggered()), this, SLOT(Disconnect()));
    ptb->addAction(act);
    addToolBar(Qt::TopToolBarArea, ptb);

    ptb = new QToolBar("Work ToolBar");

    act = new QAction("Monitor");
    act->setText("Monitoring");
    act->setShortcut(QKeySequence("CTRL+M"));
    act->setToolTip("Start monitoring\nCTRL+M");
    connect(act, SIGNAL(triggered()), work, SLOT(Monitor()));
    ptb->addAction(act);
    ptb->addSeparator();

    act = new QAction("Start");
    act->setText("Start");
    act->setShortcut(QKeySequence("CTRL+S"));
    act->setToolTip("Start measuring\nCTRL+S");
    connect(act, SIGNAL(triggered()), work, SLOT(Measure()));
    ptb->addAction(act);
    ptb->addSeparator();

    act = new QAction("Stop");
    act->setText("Stop");
    act->setShortcut(QKeySequence("CTRL+Z"));
    act->setToolTip("Stop measuring\nCTRL+Z");
    connect(act, SIGNAL(triggered()), work, SLOT(Stop()));
    ptb->addAction(act);

    addToolBar(Qt::TopToolBarArea, ptb);

    scene->addItem(work->getBoardItem());
    scene->setSceneRect(work->getBoundingRect());
    
    scale();
}
//@brief Обновить доступные com-порты
void MainWindow::refreshPort()
{
    ddlPort->clear();
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& portInfo : serialPortInfos) {
        ddlPort->insertItem(portInfo.productIdentifier(), portInfo.portName());
    }
}
//@brief Подлючится к указанному com-порту
void MainWindow::Connect()
{
    work->Connect(ddlPort->currentText());
}
//@brief Отключиться от com-порта
void MainWindow::Disconnect()
{
    work->Disconnect();
    actConnect->setEnabled(true);
}
//@brief Проверить статус подключения
void MainWindow::statusConnect(bool connected)
{
    actConnect->setEnabled(!connected);
}
//@brief Изменение размера окна
void MainWindow::scale()
{
    view->resetTransform();
    QRectF viewport = view->rect();
    qreal  multW = (viewport.width() - 10) / scene->sceneRect().width();
    qreal  multH = (viewport.height() - 10) / scene->sceneRect().height();
    view->scale(multW, multH);
}
//@brief Ивент изменения размера окна
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    scale();
}

MainWindow::~MainWindow()
{
    work->Disconnect();
    delete work;
}