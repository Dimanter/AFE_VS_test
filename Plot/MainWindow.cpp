#include "MainWindow.h"
#include <QtCharts/QChartView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QChartView* view = new QChartView;
}

MainWindow::~MainWindow()
{}
