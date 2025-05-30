#ifndef COEFOUT_H
#define COEFOUT_H

#include <QDialog>
#include <QDialogButtonBox>

#include "coefOutService.hpp"

namespace Ui
{
    class qCoefOut;
}

class qCoefOut : public QDialog, public coefOutService
{
    Q_OBJECT

public:
    qCoefOut(Service::eventCallback_t, addressing_t, QWidget *parent = nullptr);

    void updateUI();
    void updateData();

    ~qCoefOut();

protected:
    void showEvent(QShowEvent *) override;

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::qCoefOut *ui;
};

#endif // COEFOUT_H
