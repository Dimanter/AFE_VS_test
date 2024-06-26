#include "coefin.hpp"
#include "ui_coefin.h"

qCoefIn::qCoefIn(Service::eventCallback_t call, addressing_t serv, QWidget* parent) : QDialog(parent), coefInService(call, serv), ui(new Ui::qCoefIn)
{
    ui->setupUi(this);
}

void qCoefIn::updateUI()
{
    ui->coefs->setItem(0, 0, new QTableWidgetItem(QString::number(*Volt_1_8_a)));
    ui->coefs->setItem(1, 0, new QTableWidgetItem(QString::number(*Volt_1_8_b)));
    ui->coefs->setItem(2, 0, new QTableWidgetItem(QString::number(*Phase_1_8)));
    ui->coefs->setItem(0, 1, new QTableWidgetItem(QString::number(*Volt_1_4_a)));
    ui->coefs->setItem(1, 1, new QTableWidgetItem(QString::number(*Volt_1_4_b)));
    ui->coefs->setItem(2, 1, new QTableWidgetItem(QString::number(*Phase_1_4)));
    ui->coefs->setItem(0, 2, new QTableWidgetItem(QString::number(*Volt_1_2_a)));
    ui->coefs->setItem(1, 2, new QTableWidgetItem(QString::number(*Volt_1_2_b)));
    ui->coefs->setItem(2, 2, new QTableWidgetItem(QString::number(*Phase_1_2)));
    ui->coefs->setItem(0, 3, new QTableWidgetItem(QString::number(*Volt_001_a)));
    ui->coefs->setItem(1, 3, new QTableWidgetItem(QString::number(*Volt_001_b)));
    ui->coefs->setItem(2, 3, new QTableWidgetItem(QString::number(*Phase_001)));
    ui->coefs->setItem(0, 4, new QTableWidgetItem(QString::number(*Volt_002_a)));
    ui->coefs->setItem(1, 4, new QTableWidgetItem(QString::number(*Volt_002_b)));
    ui->coefs->setItem(2, 4, new QTableWidgetItem(QString::number(*Phase_002)));
    ui->coefs->setItem(0, 5, new QTableWidgetItem(QString::number(*Volt_004_a)));
    ui->coefs->setItem(1, 5, new QTableWidgetItem(QString::number(*Volt_004_b)));
    ui->coefs->setItem(2, 5, new QTableWidgetItem(QString::number(*Phase_004)));
    ui->coefs->setItem(0, 6, new QTableWidgetItem(QString::number(*Volt_008_a)));
    ui->coefs->setItem(1, 6, new QTableWidgetItem(QString::number(*Volt_008_b)));
    ui->coefs->setItem(2, 6, new QTableWidgetItem(QString::number(*Phase_008)));
    ui->coefs->setItem(0, 7, new QTableWidgetItem(QString::number(*Volt_016_a)));
    ui->coefs->setItem(1, 7, new QTableWidgetItem(QString::number(*Volt_016_b)));
    ui->coefs->setItem(2, 7, new QTableWidgetItem(QString::number(*Phase_016)));
    ui->coefs->setItem(0, 8, new QTableWidgetItem(QString::number(*Volt_032_a)));
    ui->coefs->setItem(1, 8, new QTableWidgetItem(QString::number(*Volt_032_b)));
    ui->coefs->setItem(2, 8, new QTableWidgetItem(QString::number(*Phase_032)));
    ui->coefs->setItem(0, 9, new QTableWidgetItem(QString::number(*Volt_064_a)));
    ui->coefs->setItem(1, 9, new QTableWidgetItem(QString::number(*Volt_064_b)));
    ui->coefs->setItem(2, 9, new QTableWidgetItem(QString::number(*Phase_064)));
    ui->coefs->setItem(0, 10, new QTableWidgetItem(QString::number(*Volt_128_a)));
    ui->coefs->setItem(1, 10, new QTableWidgetItem(QString::number(*Volt_128_b)));
    ui->coefs->setItem(2, 10, new QTableWidgetItem(QString::number(*Phase_128)));
}

void qCoefIn::updateData()
{
    Volt_1_8_a = ui->coefs->item(0, 0)->text().toFloat();
    Volt_1_8_b = ui->coefs->item(1, 0)->text().toFloat();
    Volt_1_4_a = ui->coefs->item(0, 1)->text().toFloat();
    Volt_1_4_b = ui->coefs->item(1, 1)->text().toFloat();
    Volt_1_2_a = ui->coefs->item(0, 2)->text().toFloat();
    Volt_1_2_b = ui->coefs->item(1, 2)->text().toFloat();
    Volt_001_a = ui->coefs->item(0, 3)->text().toFloat();
    Volt_001_b = ui->coefs->item(1, 3)->text().toFloat();
    Volt_002_a = ui->coefs->item(0, 4)->text().toFloat();
    Volt_002_b = ui->coefs->item(1, 4)->text().toFloat();
    Volt_004_a = ui->coefs->item(0, 5)->text().toFloat();
    Volt_004_b = ui->coefs->item(1, 5)->text().toFloat();
    Volt_008_a = ui->coefs->item(0, 6)->text().toFloat();
    Volt_008_b = ui->coefs->item(1, 6)->text().toFloat();
    Volt_016_a = ui->coefs->item(0, 7)->text().toFloat();
    Volt_016_b = ui->coefs->item(1, 7)->text().toFloat();
    Volt_032_a = ui->coefs->item(0, 8)->text().toFloat();
    Volt_032_b = ui->coefs->item(1, 8)->text().toFloat();
    Volt_064_a = ui->coefs->item(0, 9)->text().toFloat();
    Volt_064_b = ui->coefs->item(1, 9)->text().toFloat();
    Volt_128_a = ui->coefs->item(0, 10)->text().toFloat();
    Volt_128_b = ui->coefs->item(1, 10)->text().toFloat();
    Phase_1_8 = ui->coefs->item(2, 0)->text().toFloat();
    Phase_1_4 = ui->coefs->item(2, 1)->text().toFloat();
    Phase_1_2 = ui->coefs->item(2, 2)->text().toFloat();
    Phase_001 = ui->coefs->item(2, 3)->text().toFloat();
    Phase_002 = ui->coefs->item(2, 4)->text().toFloat();
    Phase_004 = ui->coefs->item(2, 5)->text().toFloat();
    Phase_008 = ui->coefs->item(2, 6)->text().toFloat();
    Phase_016 = ui->coefs->item(2, 7)->text().toFloat();
    Phase_032 = ui->coefs->item(2, 8)->text().toFloat();
    Phase_064 = ui->coefs->item(2, 9)->text().toFloat();
    Phase_128 = ui->coefs->item(2, 10)->text().toFloat();
}

void qCoefIn::showEvent(QShowEvent* event)
{
    updateUI();
    QDialog::showEvent(event);
}

void qCoefIn::on_buttonBox_clicked(QAbstractButton* button)
{

}

qCoefIn::~qCoefIn()
{
    delete ui;
}
