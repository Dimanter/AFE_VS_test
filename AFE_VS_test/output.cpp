#include "output.hpp"
#include "ui_output.h"

qOutput::qOutput(Service::eventCallback_t call, addressing_t serv, QWidget* parent) : QDialog(parent), outService(call, serv), ui(new Ui::qOutput)
{
    ui->setupUi(this);
}

void qOutput::showEvent(QShowEvent* event)
{
    ui->spinOut->setValue(outP);
    ui->sliderOut->setValue(outP);
    ui->spinMean->setValue(meanP - meanN);
    ui->sliderMean->setValue(meanP - meanN);
    ui->spinFreq->setValue(freq);
    ui->sliderFreq->setValue(freq);

    QDialog::showEvent(event);
}

void qOutput::hideEvent(QHideEvent* event)
{
    outP = ui->spinOut->value();
    meanP = (ui->spinOut->value() + ui->spinMean->value()) / 2.f;
    phaseP = 0.f;
    outN = ui->spinOut->value();
    meanN = (ui->spinOut->value() + ui->spinMean->value()) / 2.f;
    phaseN = 3.141592653f;
    freq = ui->spinFreq->value();

    QWidget::hideEvent(event);
}

void qOutput::on_sliderOut_valueChanged(int value)
{
    ui->spinOut->setValue(value);
}

void qOutput::on_spinOut_valueChanged(int value)
{
    ui->sliderOut->setValue(value);
}

void qOutput::on_sliderMean_valueChanged(int value)
{
    ui->spinMean->setValue(value);
}

void qOutput::on_spinMean_valueChanged(int value)
{
    ui->sliderMean->setValue(value);
}

void qOutput::on_sliderFreq_valueChanged(int value)
{
    ui->spinFreq->setValue(value);
}

void qOutput::on_spinFreq_valueChanged(int value)
{
    ui->sliderFreq->setValue(value);
}

qOutput::~qOutput()
{
    delete ui;
}
