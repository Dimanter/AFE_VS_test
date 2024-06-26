#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QGraphicsView>
#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>

#include "Work.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString&, QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void statusConnect(bool);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void Connect();
    void Disconnect();
    void refreshPort();

private:
    void scale(void);

private:
    QGraphicsScene* scene;
    QGraphicsView* view;
    QAction* actConnect;
    QComboBox* ddlPort;
    QLineEdit* text;
    Work* work;
};
#endif // MAINWINDOW_HPP
