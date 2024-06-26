#include <QSerialPortInfo>
#include <QtWidgets>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    scene = new QGraphicsScene();
    view = new QGraphicsView(scene);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setCentralWidget(view);

    work = new Work(2400, 1400);
    connect(work, &Work::portStatus, this, &MainWindow::statusConnect);

    QToolBar* ptb = new QToolBar("Control ToolBar");

    ddlPort = new QComboBox;
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& portInfo : serialPortInfos) {
        ddlPort->insertItem(portInfo.productIdentifier(), portInfo.portName());
    }
    ptb->addWidget(ddlPort);

    QAction* act = new QAction("Refresh");
    act->setText("Refresh");
    act->setShortcut(QKeySequence("CTRL+R"));
    act->setToolTip("Refresh com ports\nCTRL+R");
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
    act->setToolTip("Start monitoring (CTRL+M)");
    connect(act, SIGNAL(triggered()), work, SLOT(Monitor()));
    ptb->addAction(act);
    ptb->addSeparator();

    act = new QAction("Stop");
    act->setText("Stop");
    act->setShortcut(QKeySequence("CTRL+Z"));
    act->setToolTip("Stop measure\nCTRL+Z");
    connect(act, SIGNAL(triggered()), work, SLOT(Stop()));
    ptb->addAction(act);
    ptb->addSeparator();

    act = new QAction("Reset");
    act->setText("Reset");
    act->setShortcut(QKeySequence("CTRL+R"));
    act->setToolTip("Reser encoder\nCTRL+R");
    connect(act, SIGNAL(triggered()), work, SLOT(Reset()));
    ptb->addAction(act);
    addToolBar(Qt::TopToolBarArea, ptb);

    ptb = new QToolBar("Settings ToolBar");
    act = new QAction("Setting");
    act->setText("Settings");
    act->setShortcut(QKeySequence("CTRL+S"));
    act->setToolTip("Open settings\nCTRL+S");
    connect(act, SIGNAL(triggered()), work, SLOT(Settings()));
    ptb->addAction(act);
    addToolBar(Qt::TopToolBarArea, ptb);

    ptb = new QToolBar("Data ToolBar");
    act = new QAction("Save");
    act->setText("Save");
    act->setToolTip("Save data");
    connect(act, SIGNAL(triggered()), work, SLOT(Save()));
    ptb->addAction(act);
    act = new QAction("Clear");
    act->setText("Clear");
    act->setToolTip("Clear data");
    connect(act, SIGNAL(triggered()), work, SLOT(Clear()));
    ptb->addAction(act);
    addToolBar(Qt::TopToolBarArea, ptb);

    scene->addItem(work->getBoardItem());
    scene->setSceneRect(work->getBoundingRect());
    scale();
}

void MainWindow::refreshPort()
{
    ddlPort->clear();
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& portInfo : serialPortInfos) {
        ddlPort->insertItem(portInfo.productIdentifier(), portInfo.portName());
    }
}

void MainWindow::Connect()
{
    work->Connect(ddlPort->currentText());
}

void MainWindow::Disconnect()
{
    work->Disconnect();
    actConnect->setEnabled(true);
}

void MainWindow::statusConnect(bool connected)
{
    actConnect->setEnabled(!connected);
}

void MainWindow::scale()
{
    view->resetTransform();
    QRectF viewport = view->rect();
    qreal  multW = (viewport.width() - 3) / scene->sceneRect().width();
    qreal  multH = (viewport.height() - 3) / scene->sceneRect().height();
    view->scale(multW, multH);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    scale();
}

MainWindow::~MainWindow()
{

    delete work;
    delete scene;
    delete view;
}
