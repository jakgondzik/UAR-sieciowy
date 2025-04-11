#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include "symulator.h"
#include "dialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startSimulation();
    void stopSimulation();
    void resetSimulation();
    void aktualizujWykresy();
    void aktualizujParametry();
    void zmienParametryPID();
    void zmienParametryARX(std::vector<double> newA, std::vector<double> newB, int newK, double newOdchStan);
    void zmienTypSygnalu();
    void zmienParametrySygnalu();

    void on_uMAX_doubleSpinBox_valueChanged(double arg1);

    void on_uMIN_doubleSpinBox_valueChanged(double arg1);

    void on_AntiWindup_checkbox_stateChanged(int arg1);

    void on_parametryARX_pushButton_clicked();

    void on_przedSuma_radioButton_clicked();

    void on_wSumie_radioButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *simulationTimer;

    Sygnal sygnal;
    RegulatorPID regulator;
    ModelARX model;
    Sprzezenie sprzezenie;
    Symulator symulator;

    double aktualnyCzas;

    QCustomPlot *wykresWartosci;
    QCustomPlot *wykresUchyb;
    QCustomPlot *wykresSterowanie;

    void ustawWykresy();
    Dialog* m_okno;

    std::vector<double> A, B;
    int opoznienie;
    double odchStan;

};

#endif // MAINWINDOW_H



