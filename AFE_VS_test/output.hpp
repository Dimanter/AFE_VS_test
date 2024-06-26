#pragma once

#include <QDialog>

#include "outService.hpp"

namespace Ui
{
    class qOutput;
}

class qOutput : public QDialog, public outService
{
    Q_OBJECT

public:
    explicit qOutput(Service::eventCallback_t, addressing_t, QWidget* parent = nullptr);

    ~qOutput();

protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

private slots:
    void on_sliderOut_valueChanged(int);
    void on_sliderMean_valueChanged(int);
    void on_sliderFreq_valueChanged(int);

    void on_spinFreq_valueChanged(int arg1);

    void on_spinMean_valueChanged(int arg1);

    void on_spinOut_valueChanged(int arg1);

private:
    Ui::qOutput* ui;
};
