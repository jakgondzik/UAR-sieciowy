#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setParameters(std::vector<double>& A, std::vector<double>& B, int k, double odchStan)
{
    m_A = A;
    m_B = B;
    m_opoznienie = k;
    m_odchStan = odchStan;
}

std::vector<double> Dialog::getA() const {return m_A;}
std::vector<double> Dialog::getB() const {return m_B;}
int Dialog::getOpoznienie() const {return m_opoznienie;}
double Dialog::getOdchylenieStand() const {return m_odchStan;}


void Dialog::on_zatwierdz_pushButton_clicked()
{
    m_A = {ui->wspA1_doubleSpinBox->value(), ui->wspA2_doubleSpinBox->value(), ui->wspA3_doubleSpinBox->value()};
    m_B = {ui->wspB1_doubleSpinBox->value(), ui->wspB2_doubleSpinBox->value(), ui->wspB3_doubleSpinBox->value()};
    m_opoznienie = ui->opoznienie_spinBox->value();
    m_odchStan = ui->zaklocenie_doubleSpinBox->value();

    emit aktualizacjaParametrow(m_A, m_B, m_opoznienie, m_odchStan);

    accept();
}


void Dialog::on_anuluj_pushButton_clicked()
{
    reject();
}

