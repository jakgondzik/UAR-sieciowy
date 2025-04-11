#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void setParameters(std::vector<double>& A, std::vector<double>& B, int k, double odchStan);
    std::vector<double> getA() const;
    std::vector<double> getB() const;
    int getOpoznienie() const;
    double getOdchylenieStand() const;

signals:
    void aktualizacjaParametrow(std::vector<double> A, std::vector<double> B, int k, double odchStan);

private slots:
    void on_zatwierdz_pushButton_clicked();
    void on_anuluj_pushButton_clicked();

private:
    Ui::Dialog *ui;
    std::vector<double> m_A;
    std::vector<double> m_B;
    int m_opoznienie;
    double m_odchStan;
};

#endif // DIALOG_H
