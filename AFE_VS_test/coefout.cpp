#include "coefout.hpp"
#include "ui_coefout.h"

qCoefOut::qCoefOut(Service::eventCallback_t call, addressing_t serv, QWidget* parent) : QDialog(parent), coefOutService(call, serv), ui(new Ui::qCoefOut)
{
    ui->setupUi(this);
}

void qCoefOut::showEvent(QShowEvent* event)
{
    updateUI();
    QDialog::showEvent(event);
}

void qCoefOut::updateUI()
{
    ui->coefs->setItem(0, 0, new QTableWidgetItem(QString::number(*Out_x1_a)));
    ui->coefs->setItem(1, 0, new QTableWidgetItem(QString::number(*Out_x1_b)));
    ui->coefs->setItem(0, 1, new QTableWidgetItem(QString::number(*Out_x2_a)));
    ui->coefs->setItem(1, 1, new QTableWidgetItem(QString::number(*Out_x2_b)));
    ui->coefs->setItem(0, 2, new QTableWidgetItem(QString::number(*Out_x4_a)));
    ui->coefs->setItem(1, 2, new QTableWidgetItem(QString::number(*Out_x4_b)));
    ui->coefs->setItem(0, 3, new QTableWidgetItem(QString::number(*Out_x8_a)));
    ui->coefs->setItem(1, 3, new QTableWidgetItem(QString::number(*Out_x8_b)));
    ui->currentA->setText(QString::number(*IOut));
}

void qCoefOut::updateData()
{
    Out_x1_a = ui->coefs->item(0, 0)->text().toFloat();
    Out_x1_b = ui->coefs->item(1, 0)->text().toFloat();
    Out_x2_a = ui->coefs->item(0, 1)->text().toFloat();
    Out_x2_b = ui->coefs->item(1, 1)->text().toFloat();
    Out_x4_a = ui->coefs->item(0, 2)->text().toFloat();
    Out_x4_b = ui->coefs->item(1, 2)->text().toFloat();
    Out_x8_a = ui->coefs->item(0, 3)->text().toFloat();
    Out_x8_b = ui->coefs->item(1, 3)->text().toFloat();
    IOut = ui->currentA->text().toFloat();
}

void qCoefOut::on_buttonBox_clicked(QAbstractButton* button)
{

}

qCoefOut::~qCoefOut()
{
    delete ui;
}

