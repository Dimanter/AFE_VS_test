#include "device.hpp"
#include "ui_device.h"

device::device(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::device)
{
    ui->setupUi(this);
}

QString device::getMeter()
{
    return ui->Meter->text();
}

QString device::getDocument()
{
    return ui->Document->text();
}

QString device::getNumber()
{
    return ui->Number->text();
}

device::~device()
{
    delete ui;
}
