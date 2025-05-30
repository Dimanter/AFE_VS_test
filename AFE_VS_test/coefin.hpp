#ifndef COEFIN_H
#define COEFIN_H

#include <QDialog>
#include <QDialogButtonBox>

#include "coefInService.hpp"

namespace Ui
{
    class qCoefIn;
}

class qCoefIn : public QDialog, public coefInService
{
    Q_OBJECT

public:
    explicit qCoefIn(Service::eventCallback_t, addressing_t, QWidget *parent = nullptr);

    void updateUI();
    void updateData();

    ~qCoefIn();

protected:
    void showEvent(QShowEvent *) override;

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::qCoefIn *ui;
};

#endif // COEFIN_H
